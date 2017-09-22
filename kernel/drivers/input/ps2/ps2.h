#ifndef INPUT_PS2_H
#define INPUT_PS2_H

#include <sched/spinlock.h>
#include <stdint.h>
#include <stdbool.h>

struct Ps2Controller {
	struct Ps2Device *dev;
	uint32_t id;
	bool connected;

	spinlock_t *controllerLock;

	int (*sendCommand)(uint8_t command);
	int (*writeWait)(void);
	int (*readWait)(void);
	void (*write)(uint8_t data);
	uint8_t (*read)(void);
};

struct Ps2Device {
	//controller this device belongs to
	struct Ps2Controller *controller;
	const struct Ps2Driver *drv;

	int interruptCount;
};

struct Ps2Driver {
	uint32_t *IDs;
	int IDsLen;

	void (*interrupt)(struct Ps2Device *dev);

	struct Ps2Device *(*newDevice)(struct Ps2Controller *controller);
	void (*deleteDevice)(struct Ps2Device *dev);
};

void ps2RegisterDriver(const struct Ps2Driver *drv);

#endif