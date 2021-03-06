// TinyWire Slave RX
// Kevin Kuwata
// recieves byte from master, and activates relay
// Created 2/20/2018

#include <TinyWire.h> //https://github.com/lucullusTheOnly/TinyWire
#include <EEPROM.h>

#define RELAY_PIN   4


#define REGISTER_MAP_SIZE    3// ADDRESS, STATUS, ON

volatile byte SLAVE_ADDRESS  =    0x18; //default


volatile int ReceivedData[32]; //32 byte array to act as a buffer for I2C data. 32 bytes is the max for an UNO 


//Control Flags
volatile bool update_register = false;
volatile bool relay_state = false; //default off;


void setup() {
	
	//Read EEPROM, is it empty (0xFF)? or does it have a value?
	byte value =  EEPROM.read(1);
	if(value == 0xFF){
		//never been written before, USE THE Default address.
		SLAVE_ADDRESS = 0x18; //default
	}
	else{
		SLAVE_ADDRESS = EEPROM.read(1);
	}

	TinyWire.begin(SLAVE_ADDRESS);
    pinMode(RELAY_PIN, OUTPUT);
	TinyWire.onReceive(receiveEvent); // register event
    TinyWire.onRequest(onI2CRequest);
}

void loop() {
	
	if(ReceivedData[0] == 0x01){
		digitalWrite(RELAY_PIN, HIGH);
	}
	if(ReceivedData[0] == 0x00){
		digitalWrite(RELAY_PIN, LOW);
	}
	
	if(ReceivedData[0] == 0x03){

		//slave address. update SLAVE_ADDRESS
		SLAVE_ADDRESS = ReceivedData[1];
		//save to eerprom
		EEPROM.write(1, SLAVE_ADDRESS);
		TinyWire.begin(SLAVE_ADDRESS);
		
		ReceivedData[0]=0x99;
		ReceivedData[1] = 0x99;
	}
	
	
}

/*========================================================*/
//        Helper Functions
/*========================================================*/


/*========================================================*/
//        ISR
/*========================================================*/
/*
    @brief: When the master initiates a command and data to slave
		ie) the master says 0x01, then sends a 1, means command: 0x01 then the slave listens for the next thing, which is the relay state 1
    @global: This is modifying ReceivedData array, which stores all the bytes that a master sent to the slave.
	@input:
    @returns: none
    @flags:  none
*/
void receiveEvent(int bytesReceived) {
	int index = 0; //index of all the data received from master.
	
  while(TinyWire.available() > 0){ //this was the solution, 
		ReceivedData[index] = TinyWire.read();
		index++;
  }//end of the while loop, data available
}// end of receive ISR


/*
    @brief: When the master requests data from the slave, this
      ISR is triggered. 
    @input: none, 
    @returns: none
    @flags:  none
*/
void onI2CRequest() {
	//TODO: add in something to report the address.
	if(digitalRead(RELAY_PIN) == HIGH) TinyWire.send(0x01);
	else{
		TinyWire.send(0x00);
	}
}// end of request ISR
