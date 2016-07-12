#ifndef PS2_H
#define PS2_H

void ps2_send_command(char command);

char ps2_read_data(void);

void ps2_write_data(char data);

char ps2_read_status(void);

void ps2_send_port1(char data);


void ps2_init(void);

void ps2_flush_output(void);

#endif
