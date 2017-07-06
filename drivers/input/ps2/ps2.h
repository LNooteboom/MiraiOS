#ifndef INPUT_PS2_H
#define INPUT_PS2_H

#include <sched/spinlock.h>
#include <stdint.h>
#include <stdbool.h>

struct ps2Controller {
	struct ps2Device *dev;
	uint32_t id;
	bool connected;

	spinlock_t *controllerLock;

	int (*sendCommand)(uint8_t command);
	int (*writeWait)(void);
	int (*readWait)(void);
	void (*write)(uint8_t data);
	uint8_t (*read)(void);
};

struct ps2Device {
	//controller this device belongs to
	struct ps2Controller *controller;
	const struct ps2Driver *drv;

	int interruptCount;
};

struct ps2Driver {
	uint32_t *IDs;
	int IDsLen;

	void (*interrupt)(struct ps2Device *dev);

	struct ps2Device *(*newDevice)(struct ps2Controller *controller);
	void (*deleteDevice)(struct ps2Device *dev);
};

void ps2RegisterDriver(const struct ps2Driver *drv);

#endif