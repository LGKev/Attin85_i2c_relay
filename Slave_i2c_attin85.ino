// TinyWire Slave RX
// Kevin Kuwata
// recieves byte from master, and activates relay
// Created 2/20/2018

#include <TinyWire.h> //https://github.com/lucullusTheOnly/TinyWire

//#define DEBUG_OUTPUT
#define BIT0    0b00000001
#define BIT1    0b00000010


#define RELAY_PIN   4
#define LED         0 // on board led for the Attiny programmer

#define MAX_BYTES_RECEIVED 3 //we only are sending to turn ON, OFF, STATUS
// I think its 3 bytes because, you first send address
// then you send register you want to talk to
// then you send the value you want to send.

#define REGISTER_MAP_SIZE   3// ADDRESS, STATUS, ON
#define SLAVE_ADDRESS   0x01//1 //whats a good way to choose?


//Address Map for COMMANDS
#define TURN_ON_REG     (0X01)
#define STATUS_REG      (0X02)


//Control Flags
volatile bool update_register = false;
volatile bool relay_state = false; //default off;

volatile byte new_address; //latest address for the slave.

byte registerMap[REGISTER_MAP_SIZE];
volatile byte receivedCommands[MAX_BYTES_RECEIVED];


//ISR prototypes
void requestEvent(void);
void receiveEvent(void);

void setup() {

  TinyWire.begin(0x01);                // join i2c bus with address #1
  TinyWire.onReceive(receiveEvent); // register event
  TinyWire.onRequest(requestEvent);
  pinMode(RELAY_PIN, OUTPUT);

  registerMap[1] = 0; //Relay NC
  registerMap[2] = 0x00; // all cleared
  new_address = SLAVE_ADDRESS;
  digitalWrite(LED, LOW); //only place the led is turned on is in the ONReceive ISR. and turned off here.
}

void loop() {

    digitalWrite(RELAY_PIN, receivedCommands[1]);

  
  relayConfig();
  //check the flags ... polling?
  if (update_register == 1) {
    //update the stuff
    update(); // updates the relay state only.
    update_register = 0; //reset flag`

#ifdef DEBUG_OUTPUT
    Serial.print("the address of the slave is:  ");
    Serial.println(new_address);

    Serial.print("the status register is:  ");
    byte statusBitStream = registerMap[2];
    Serial.println(statusBitStream, BIN);
#endif
  }
  //check here the state of the relay, in register map
  // the following code can probably be a nice function.
  // set relay STATE accordingly.
  if (registerMap[1] == 1) {
    //TODO: add the status register update here.
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED, HIGH);
    registerMap[2] |= BIT0;
  }
  if (registerMap[1] == 0) {
    //TODO: add the status register update here
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED, LOW);
    registerMap[2] &= ~BIT0; //this should make bit0 in the map[2] to be a 0.
  }
}

/*========================================================*/
//        Helper Functions
/*========================================================*/
/*
    @brief: Parses the commands received to determine how the
      slave device should behave. this is where the address
      changes, and where the state of the relay changes
      DOES NOT update the actual relay output here, that is done in
      the main loop.
    @input: *global* receivedCommands
    @returns: nothing

    @flags:  update_register flag to 1
*/
void relayConfig() {
  //now we have collected info, now we need to parse it.
  // use a switch statement to change based on the command

  switch (receivedCommands[0]) {
    case 0x03: //change slave address
      update_register = 1;
      new_address = receivedCommands[1];
#ifdef DEBUG_OUTPUT
      Serial.print("The newest address is: ");
      Serial.println(new_address);
      for (int i = 0; i < 10; i++) {
        digitalWrite(13, HIGH);
        delay(75);
        digitalWrite(13, LOW);
        delay(75);
      }
#endif
      //QUESTION: what would happen if this was left out?, especially
      //if the number of bytes expected is larger than 2.
      //      bytesReceived--;
      //      if (bytesReceived == 1) {
      //        return; // only expecting 2 bytes
      //      }
      break;
    case 0x01: //change relay state: on or off
      //next byte is the state, 1 is on, 0 is off.
      relay_state = receivedCommands[1];
      update_register = 1;
      //      bytesReceived--;
      //      if (bytesReceived == 1) {
      //        return; // only expecting 2 bytes
      //      }
      break;
    default:
      //trying to write to a READ-ONLY register.
      digitalWrite(13, LOW);
      update_register = 0;
      return;// out of bounds
  }
}

/*
    @brief: Sets i2c address being used, updates the
      registerMap for relay state on or off
    @input: none, *global slave address*
    @returns: none
    @flags:  none
*/
void update() {
  //write to the memory register. status register?
  TinyWire.begin(new_address);
  registerMap[1] = relay_state;
}




/*========================================================*/
//        ISR
/*========================================================*/
/*
    @brief: When the master initiates a command and data to slave
    @input: bytesReceieved, to compare how many bytes received,
      with how many bytes expected. Only keep the number of expected
      and then lose the rest.
    @returns: none
    @flags:  none
*/
void receiveEvent(int bytesReceived) {
  //bug is in this ISR
  while(TinyWire.available() > 0){
    if(TinyWire.read() == 0x01){
      receivedCommands[1] = TinyWire.read();
    }
  }

}// end of receive ISR


/*
    @brief: When the master requests data from the slave, this
      ISR is triggered. Should give the master entire registerMAP.
    @input: none, *global registerMAP*
    @returns: none
    @flags:  none
*/
void requestEvent() {

  //  TinyWire.write(registerMap, REGISTER_MAP_SIZE);
  //  TinyWire.send(registerMap[0]);
  //  TinyWire.send(registerMap[1]);
  //  we will send entire map, but we only need to
  //   send the status, so bit shift?
}// end of request ISR

