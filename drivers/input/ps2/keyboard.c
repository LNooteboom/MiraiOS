#include "ps2.h"
#include <modules.h>
#include <print.h>
#include <mm/heap.h>

struct kbDevice {
	struct ps2Device ps2Dev;
	unsigned int ledState;
};

static void kbInterrupt(struct ps2Device *dev);

static struct ps2Device *kbNewDevice(struct ps2Controller *controller);

static void kbDeleteDevice(struct ps2Device *dev);

static uint32_t kbIDs[] = {
	0,
	0x020083AB
};

static const struct ps2Driver kbDriver = {
	.IDs = kbIDs,
	.IDsLen = sizeof(kbIDs) / sizeof(kbIDs[0]),
	.interrupt = kbInterrupt,
	.newDevice = kbNewDevice,
	.deleteDevice = kbDeleteDevice
};

static void kbInterrupt(struct ps2Device *dev) {
	struct kbDevice *kbDev = (struct kbDevice*)dev;
	cprint('i');
	hexprint(kbDev->ps2Dev.controller->read());
}

static struct ps2Device *kbNewDevice(struct ps2Controller *controller) {
	struct kbDevice *ret = kmalloc(sizeof(struct kbDevice));
	ret->ledState = 0;
	ret->ps2Dev.drv = &kbDriver;
	ret->ps2Dev.controller = controller;
	ret->ps2Dev.interruptCount = 0;

	//enable scanning
	controller->sendCommand(0xF4);

	return &ret->ps2Dev;
}

static void kbDeleteDevice(struct ps2Device *dev) {
	kfree(dev);
}

int kbInit(void) {
	ps2RegisterDriver(&kbDriver);

	return 0;
}

MODULE_INIT_LEVEL(kbInit, 3);