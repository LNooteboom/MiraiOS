#include "ps2.h"
#include "io.h"
#include "tty.h"

void ps2_init(void) {
	sprint("Initialising PS/2...\n", currentattrib);
	//disable devices
	ps2_send_command(0xAD); //first port
	ps2_send_command(0xA7); //second port

	ps2_flush_output();

	//set controller configuration
	ps2_send_command(0x20); //read controller output
	char config = ps2_read_data();
	char chnl2_exists = config & 0x20;
	config &= 0xBC; //clear bits 0, 1 and 6
	ps2_send_command(0x60); //write controller output
	ps2_write_data(config); //send new config

	//perform self test
	ps2_send_command(0xAA);
	if (ps2_read_data() != 0x55) {
		sprint("PS/2 self test failed!", currentattrib);
		while (1) {}
	}

	//check if there are 2 channels
	if (chnl2_exists != 0) {
		//possibly 2 channels
		ps2_send_command(0xA8);
		ps2_send_command(0x20);
		char config = ps2_read_data();
		if ((config & 0x20) == 0) {
			//dual channel
			chnl2_exists = 1;
		} else {
			//single channel
			chnl2_exists = 0;
		}
	} else {
		//single channel
		chnl2_exists = 0;
	}

	//perform interface checks
	//check port 1
	ps2_send_command(0xAB);
	if (ps2_read_data() == 0) {
		sprint("PS/2 port 1 test complete.\n", currentattrib);
	} else {
		sprint("PS/2 port 1 test failed.\n", currentattrib);
	}
	//check port 2, if it exists
	if (chnl2_exists) {
		ps2_send_command(0xA9);
		if (ps2_read_data() == 0) {
			sprint("PS/2 port 2 test complete.\n", currentattrib);
		} else {
			sprint("PS/2 port 2 test failed.\n", currentattrib);
		}
	}

	//enable devices
	ps2_send_command(0xAE);
	if (chnl2_exists) {
		ps2_send_command(0xA8);
		ps2_send_command(0x20);
		char config = ps2_read_data();
		config |=0x03; //turn on interrupts for port 1+2
		ps2_send_command(0x60);
		ps2_write_data(config);
	} else {
		ps2_send_command(0x20);
		char config = ps2_read_data();
		config |= 0x01; //turn on interrupts on port 1
		config &= 0xFD; //turn off interrupts on port 2
		ps2_send_command(0x60);
		ps2_write_data(config);
	}

	//unmask interrupt
	//char mask = pic_getmask_master();
	//mask |= 0x02; //set
	//pic_setmask_master(mask);
	asm ("sti");
	ps2_send_port1(0xFF);
}

void ps2_flush_output(void) {
	while ((ps2_read_status() & 0x01) == 1){
		inb(0x60);
	}
}
