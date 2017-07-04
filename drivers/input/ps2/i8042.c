#include <modules.h>
#include <stdbool.h>
#include <io.h>
#include <sched/sleep.h>
#include <errno.h>
#include <irq.h>
#include <print.h>
#include <sched/spinlock.h>

#define I8042_PORT_DATA		0x60
#define I8042_PORT_STATUS	0x64
#define I8042_PORT_CMD		0x64

#define I8042_TIMEOUT		0xFFFF

#define I8042_BUFFER_SIZE	16

#define I8042_IRQ_PORT1		1

#define I8042_STATUS_OUTBUFFER	(1 << 0)
#define I8042_STATUS_INBUFFER	(1 << 1)
#define I8042_STATUS_COMMAND	(1 << 3)
#define I8042_STATUS_TIMEOUT	(1 << 6)
#define I8042_STATUS_PARITY		(1 << 7)

#define I8042_CONFIG_IRQ_PORT1	(1 << 0)
#define I8042_CONFIG_IRQ_PORT2	(1 << 1)
#define I8042_CONFIG_DIS_PORT1	(1 << 4)
#define I8042_CONFIG_XLATE		(1 << 6)

#define I8042_CMD_READ_CONFIG		0x20
#define I8042_CMD_WRITE_CONFIG		0x60
#define I8042_CMD_SELF_TEST			0xAA
#define I8042_CMD_TEST_PORT1		0xAB
#define I8042_CMD_DISABLE_PORT1		0xAD
#define I8042_CMD_DISABLE_PORT2		0xA7
#define I8042_CMD_ENABLE_PORT1		0xAE

static spinlock_t i8042Lock;

static bool i8042CmdSent = true;

static void i8042Error(const char *msg) {
	kprintf("[i8042] %s\n", msg);
}

static inline char i8042Status(void) {
	return in8(I8042_PORT_STATUS);
}

static inline int i8042WriteWait(void) {
	int timeout = 0;
	do {
		if (!(i8042Status() & I8042_STATUS_INBUFFER)) {
			break;
		}
		relax();
	} while (++timeout < I8042_TIMEOUT);
	if (timeout >= I8042_TIMEOUT) {
		i8042Error("Write timed out");
		return -EIO;
	}
	return 0;
}

static inline int i8042ReadWait(void) {
	int timeout = 0;
	do {
		if (i8042Status() & I8042_STATUS_OUTBUFFER) {
			break;
		}
		relax();
	} while (++timeout < I8042_TIMEOUT);
	if (timeout >= I8042_TIMEOUT) {
		i8042Error("Read timed out");
		return -EIO;
	}
	return 0;
}

static inline int i8042ReadData(char *data) {
	int error = i8042ReadWait();
	if (error)
		return error;

	*data = in8(I8042_PORT_DATA);
	return 0;
}

static inline int i8042WriteData(char data) {
	int error = i8042WriteWait();
	if (error)
		return error;

	out8(I8042_PORT_DATA, data);
	return 0;
}

static inline int i8042Cmd(char cmd) {
	int error = i8042WriteWait();
	if (error)
		return error;

	out8(I8042_PORT_CMD, cmd);
	return 0;
}

static int i8042Flush(void) {
	acquireSpinlock(&i8042Lock);
	int count = 0;
	int ret = 0;
	while (i8042Status() & I8042_STATUS_OUTBUFFER) {
		if (count++ < I8042_BUFFER_SIZE) {
			in8(I8042_PORT_DATA);
		} else {
			ret = -EIO;
			break;
		}
	}
	releaseSpinlock(&i8042Lock);
	return ret;
}

static void i8042Interrupt(void) {
	char t;
	i8042ReadData(&t);
	sprint("ps2 interrupt!\n");
}

int i8042Init(void) {
	int error;

	//check if exists
	error = i8042Flush();
	if (error) {
		i8042Error("Device not found");
		return error;
	}

	//Disable devices
	error = i8042Cmd(I8042_CMD_DISABLE_PORT1);
	if (error) {
		i8042Error("Error disabling port 1");
		return error;
	}
	i8042Cmd(I8042_CMD_DISABLE_PORT2);

	//Flush output buffer
	error = i8042Flush();
	if (error) {
		i8042Error("Error flushing buffer");
		return error;
	}

	//Set controller config byte
	error = i8042Cmd(I8042_CMD_READ_CONFIG);
	if (error) {
		return error;
	}
	char config;
	error = i8042ReadData(&config);
	if (error) {
		return error;
	}
	config &= ~(I8042_CONFIG_IRQ_PORT1 | I8042_CONFIG_IRQ_PORT2 | I8042_CONFIG_XLATE);
	error = i8042Cmd(I8042_CMD_WRITE_CONFIG);
	if (error) {
		return error;
	}
	error = i8042WriteData(config);
	if (error) {
		return error;
	}

	//Perform self-test
	error = i8042Cmd(I8042_CMD_SELF_TEST);
	if (error) {
		return error;
	}
	char response = 0;
	error = i8042ReadData(&response);
	if (response != 0x55) {
		i8042Error("Controller self-test failed");
	}

	//Test ps2 port 1
	error = i8042Cmd(I8042_CMD_TEST_PORT1);
	if (error) {
		return error;
	}
	error = i8042ReadData(&response);
	if (error) {
		return error;
	}
	if (response) {
		i8042Error("No keyboard connected");
	}

	//Enable port 1
	error = i8042Cmd(I8042_CMD_ENABLE_PORT1);
	if (error) {
		return error;
	}

	//Enable interrupts
	interrupt_t vec = allocIrqVec();
	if (!vec) {
		return -EBUSY;
	}
	error = routeInterrupt(i8042Interrupt, vec, 0, "PS/2 Keyboard");
	if (error) {
		deallocIrqVec(vec);
		return error;
	}
	error = routeIrqLine(vec, I8042_IRQ_PORT1, HWIRQ_FLAG_ISA);
	if (error) {
		unrouteInterrupt(vec);
		deallocIrqVec(vec);
		return error;
	}

	error = i8042Cmd(I8042_CMD_WRITE_CONFIG);
	if (error) {
		return error;
	}
	error = i8042WriteData((config | I8042_CONFIG_IRQ_PORT1) & ~I8042_CONFIG_DIS_PORT1);
	if (error) {
		return error;
	}

	//reset
	error = i8042WriteData(0xFF);
	if (error) {
		return error;
	}

	return 0;
}

MODULE_INIT(i8042Init);