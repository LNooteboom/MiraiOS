#include "main.h"

#include <global.h>
#include <pio.h>

#define PS2COMMAND_READCONFIG 0x20
#define PS2COMMAND_WRITECONFIG 0x60

#define PS2COMMAND_DISABLEPORT1 0xAD
#define PS2COMMAND_ENABLEPORT1 0xAE
#define PS2COMMAND_DISABLEPORT2 0xA7
#define PS2COMMAND_ENABLEPORT2 0xA8

#define PS2COMMAND_TESTCONTROLLER 0xAA
#define PS2COMMAND_TESTPORT1 0xAB
#define PS2COMMAND_TESTPORT2 0xA9

#define PS2COMMAND_READCONTROLLEROUTPUT 0xD0
#define PS2COMMAND_WRITECONTROLLEROUTPUT 0xD1


void ps2Init(void) {
	//sprint("Initialising PS/2...\n");
	//disable devices
	ps2SendCommand(PS2COMMAND_DISABLEPORT1); //first port
	ps2SendCommand(PS2COMMAND_DISABLEPORT2); //second port

	ps2FlushOutput();

	//set controller configuration
	ps2SendCommand(PS2COMMAND_READCONFIG); //read controller output
	uint8_t config = ps2ReadData();
	bool chnl2Exists = config & 0x20;
	config &= 0xBC; //clear bits 0, 1 and 6
	ps2SendCommand(PS2COMMAND_WRITECONFIG); //write controller output
	ps2WriteData(config); //send new config

	//perform self test
	ps2SendCommand(PS2COMMAND_TESTCONTROLLER);
	if (ps2ReadData() != 0x55) {
		//sprint("PS/2 self test failed!");
		while (1) {}
	}

	//check if there are 2 channels
	if (chnl2Exists) {
		//possibly 2 channels
		ps2SendCommand(PS2COMMAND_ENABLEPORT2);
		ps2SendCommand(PS2COMMAND_READCONFIG);
		config = ps2ReadData();
		if (!(config & 0x20)) {
			//dual channel
			chnl2Exists = true;
		} else {
			//single channel
			ps2SendCommand(PS2COMMAND_DISABLEPORT2);
			chnl2Exists = false;
		}
	} else {
		//single channel
		chnl2Exists = false;
	}

	//perform interface checks
	//check port 1
	ps2SendCommand(PS2COMMAND_TESTPORT1);
	if (ps2ReadData() == 0) {
		//sprint("PS/2 port 1 test complete.\n");
	} else {
		//sprint("PS/2 port 1 test failed.\n");
	}
	//check port 2, if it exists
	if (chnl2Exists) {
		ps2SendCommand(PS2COMMAND_TESTPORT2);
		if (ps2ReadData() == 0) {
			//sprint("PS/2 port 2 test complete.\n");
		} else {
			//sprint("PS/2 port 2 test failed.\n");
		}
	}

	//enable devices
	ps2SendCommand(PS2COMMAND_ENABLEPORT1);

	ps2SendCommand(PS2COMMAND_READCONFIG);
	config = ps2ReadData();
	if (chnl2Exists) {
		config |=0x03; //turn on interrupts for port 1+2
	} else {
		config |= 0x01; //turn on interrupts on port 1
		config &= 0xFD; //turn off interrupts on port 2
	}
	ps2SendCommand(PS2COMMAND_WRITECONFIG);
	ps2WriteData(config);

	ps2SendPort1(0xFF);
}

void ps2FlushOutput(void) {
	while ((ps2ReadStatus() & 0x01) == 1){
		inb(0x60);
	}
}
