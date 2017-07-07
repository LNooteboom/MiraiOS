#include "ps2.h"
#include <modules.h>
#include <drivers/input/eventcodes.h>
#include <stdbool.h>
#include <print.h>
#include <mm/heap.h>

#define SCANCODE_MODIFIER	0xE0
#define SCANCODE_RELEASED	0xF0

struct kbDevice {
	struct ps2Device ps2Dev;

	bool keyReleased;
	bool keyModifier;
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

static uint16_t kbScanTable[] = {
	/*00*/ KEY_RESERVED,	KEY_F9,			KEY_RESERVED,	KEY_F5,			KEY_F3,			KEY_F1,			KEY_F2,			KEY_F12,
	/*08*/ KEY_RESERVED,	KEY_F10,		KEY_F8,			KEY_F6,			KEY_F4,			KEY_TAB,		KEY_GRAVE,		KEY_RESERVED,
	/*10*/ KEY_RESERVED,	KEY_LEFTALT,	KEY_LEFTSHIFT,	KEY_RESERVED,	KEY_LEFTCTRL,	KEY_Q,			KEY_1,			KEY_RESERVED,
	/*18*/ KEY_RESERVED,	KEY_RESERVED,	KEY_Z,			KEY_S,			KEY_A,			KEY_W,			KEY_2,			KEY_RESERVED,
	/*20*/ KEY_RESERVED,	KEY_C,			KEY_X,			KEY_D,			KEY_E,			KEY_4,			KEY_3,			KEY_RESERVED,
	/*28*/ KEY_RESERVED,	KEY_SPACE,		KEY_V,			KEY_F,			KEY_T,			KEY_R,			KEY_5,			KEY_RESERVED,
	/*30*/ KEY_RESERVED,	KEY_N,			KEY_B,			KEY_H,			KEY_G,			KEY_Y,			KEY_6,			KEY_RESERVED,
	/*38*/ KEY_RESERVED,	KEY_RESERVED,	KEY_M,			KEY_J,			KEY_U,			KEY_7,			KEY_8,			KEY_RESERVED,
	/*40*/ KEY_RESERVED,	KEY_COMMA,		KEY_K,			KEY_I,			KEY_O,			KEY_0,			KEY_9,			KEY_RESERVED,
	/*48*/ KEY_RESERVED,	KEY_DOT,		KEY_SLASH,		KEY_L,			KEY_SEMICOLON,	KEY_P,			KEY_MINUS,		KEY_RESERVED,
	/*50*/ KEY_RESERVED,	KEY_RESERVED,	KEY_APOSTROPHE,	KEY_RESERVED,	KEY_LEFTBRACE,	KEY_EQUALS,		KEY_RESERVED,	KEY_RESERVED,
	/*58*/ KEY_CAPSLOCK,	KEY_RIGHTSHIFT,	KEY_ENTER,		KEY_RIGHTBRACE,	KEY_RESERVED,	KEY_BACKSLASH,	KEY_RESERVED,	KEY_RESERVED,
	/*60*/ KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_BACKSPACE,	KEY_RESERVED,
	/*68*/ KEY_RESERVED,	KEY_KP1,		KEY_RESERVED,	KEY_KP4,		KEY_KP7,		KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,
	/*70*/ KEY_KP0,			KEY_KPDOT,		KEY_KP2,		KEY_KP5,		KEY_KP6,		KEY_KP8,		KEY_ESC,		KEY_NUMLOCK,
	/*78*/ KEY_F11,			KEY_KPPLUS,		KEY_KP3,		KEY_KPMINUS,	KEY_KPASTERISK,	KEY_KP9,		KEY_SCROLLLOCK,	KEY_RESERVED,
	/*80*/ KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,
	/*88*/ KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,
	/*90*/ KEY_SEARCH,		KEY_RIGHTALT,	KEY_RESERVED,	KEY_RESERVED,	KEY_RIGHTCTRL,	KEY_PREVIOUSSONG, KEY_RESERVED,	KEY_RESERVED,
	/*98*/ KEY_FAVORITES,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_MENU,
	/*A0*/ KEY_REFRESH,		KEY_VOLUMEDOWN,	KEY_RESERVED,	KEY_MUTE,		KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_MENU,
	/*A8*/ KEY_STOP,		KEY_RESERVED,	KEY_RESERVED,	KEY_CALCULATOR,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_APPS,
	/*B0*/ KEY_FORWARD, 	KEY_RESERVED,	KEY_VOLUMEUP,	KEY_RESERVED,	KEY_PLAYPAUSE,	KEY_RESERVED,	KEY_RESERVED,	KEY_POWER,
	/*B8*/ KEY_BACK,		KEY_RESERVED,	KEY_HOMEPAGE,	KEY_STOP,		KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_SLEEP,
	/*C0*/ KEY_COMPUTER,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,
	/*C8*/ KEY_EMAIL,		KEY_RESERVED,	KEY_KPSLASH,	KEY_RESERVED,	KEY_RESERVED,	KEY_NEXTSONG,	KEY_RESERVED,	KEY_RESERVED,
	/*D0*/ KEY_SELECT,		KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,
	/*D8*/ KEY_RESERVED,	KEY_RESERVED,	KEY_KPENTER,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_WAKE,		KEY_RESERVED,
	/*E0*/ KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,
	/*E8*/ KEY_RESERVED,	KEY_END,		KEY_RESERVED,	KEY_LEFT,		KEY_HOME,		KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,
	/*F0*/ KEY_INSERT,		KEY_DELETE,		KEY_DOWN,		KEY_RESERVED,	KEY_RIGHT,		KEY_UP,			KEY_RESERVED,	KEY_RESERVED,
	/*F8*/ KEY_RESERVED,	KEY_RESERVED,	KEY_PAGEDOWN,	KEY_RESERVED,	KEY_RESERVED,	KEY_PAGEUP,		KEY_RESERVED,	KEY_RESERVED,

	//extra row for scancodes >= 0x80 without modifier
	/*00*/ KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_F7,			KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED,	KEY_RESERVED
};

static void kbInterrupt(struct ps2Device *dev) {
	struct kbDevice *kbDev = (struct kbDevice*)dev;
	uint16_t data = kbDev->ps2Dev.controller->read();
	uint16_t keycode;
	
	switch (data) {
		case SCANCODE_MODIFIER:
			kbDev->keyModifier = true;
			break;
		case SCANCODE_RELEASED:
			kbDev->keyReleased = true;
			break;
		default:
			if (kbDev->keyModifier) {
				data += 0x80;
				kbDev->keyModifier = false;
			} else if (data >= 0x80) {
				data += (0x100 - 0x80);
			}
			keycode = (data < (sizeof(kbScanTable) / sizeof(kbScanTable[0]) )) ? kbScanTable[data] : KEY_RESERVED;
			cprint('i');
			decprint(keycode);

			kbDev->keyReleased = false;
			break;
	}
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