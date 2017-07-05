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

static inline uint8_t i8042Status(void) {
	return in8(I8042_PORT_STATUS);
}

static int i8042WriteWait(void) {
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

static int i8042ReadWait(void) {
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

static inline uint8_t i8042ReadData(void) {
	return in8(I8042_PORT_DATA);
}

static inline void i8042WriteData(uint8_t data) {
	out8(I8042_PORT_DATA, data);
}

static inline void i8042Cmd(uint8_t cmd) {
	out8(I8042_PORT_CMD, cmd);
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

static int i8042SendPort1(uint8_t command) {
	int ret = 0;
	do {
		if (i8042WriteWait())
			return -EIO;
		i8042WriteData(command);
		//get return value
		if (i8042ReadWait())
			return -EIO;
		ret = i8042ReadData();
	} while (ret == 0xFE);
	return ret;
}

static void i8042Interrupt(void) {
	if (!i8042CmdSent) {
		//call keyboard driver
		cprint('k');
		hexprintln(i8042ReadData());
	}
}

int i8042Init(void) {
	//check if exists
	if (i8042Flush()) {
		i8042Error("Device not found");
		return -EIO;
	}

	//Disable devices
	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_DISABLE_PORT1);
	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_DISABLE_PORT2);

	//Flush output buffer
	if (i8042Flush()) {
		i8042Error("Error flushing buffer");
		return -EIO;
	}

	//Set controller config byte
	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_READ_CONFIG);
	if (i8042ReadWait())
		return -EIO;
	uint8_t config = i8042ReadData();
	config &= ~(I8042_CONFIG_IRQ_PORT1 | I8042_CONFIG_IRQ_PORT2 | I8042_CONFIG_XLATE);
	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_WRITE_CONFIG);
	if (i8042WriteWait())
		return -EIO;
	i8042WriteData(config);

	//Perform self-test
	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_SELF_TEST);
	if (i8042ReadWait())
		return -EIO;
	if (i8042ReadData() != 0x55) {
		i8042Error("Controller self-test failed, continuing anyway...");
	}

	//Test ps2 port 1
	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_TEST_PORT1);
	if (i8042ReadWait())
		return -EIO;
	if (i8042ReadData()) {
		i8042Error("Port 1 self test failed");
	}

	//Enable port 1
	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_ENABLE_PORT1);

	//Enable interrupts
	interrupt_t vec = allocIrqVec();
	if (!vec)
		return -EBUSY;
	if (routeInterrupt(i8042Interrupt, vec, 0, "PS/2 Keyboard")) {
		deallocIrqVec(vec);
		return -EIO;
	}
	if (routeIrqLine(vec, I8042_IRQ_PORT1, HWIRQ_FLAG_ISA)) {
		unrouteInterrupt(vec);
		deallocIrqVec(vec);
		return -EIO;
	}
	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_READ_CONFIG);
	if (i8042ReadWait())
		return -EIO;
	config = i8042ReadData();

	config |= I8042_CONFIG_IRQ_PORT1;

	if (i8042WriteWait())
		return -EIO;
	i8042Cmd(I8042_CMD_WRITE_CONFIG);
	if (i8042WriteWait())
		return -EIO;
	i8042WriteData(config);

	//reset
	if (i8042SendPort1(0xFF) < 0) {
		i8042Error("No device on port 1");
		return -EIO;
	}
	if (i8042ReadWait())
		return -EIO;
	if (i8042ReadData() != 0xAA) {
		i8042Error("Device on port 1 self-test failed!");
	}

	i8042CmdSent = false;

	return 0;
}

MODULE_INIT(i8042Init);