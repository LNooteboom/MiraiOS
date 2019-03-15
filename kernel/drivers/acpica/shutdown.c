#include <uapi/syscalls.h>
#include <sched/process.h>
#include <sched/smpcall.h>
#include <errno.h>
#include <panic.h>
#include <io.h>
#include <print.h>
#include <config.h>

extern int kernelPrepareShutdown(int mode);
extern void fbPanicUpdate(void);

static __attribute__((noreturn)) void doReboot(void) {
	//reset via i8042
	uint8_t good = 0x02;
	while (good & 0x02)
		good = in8(0x64);
	out8(0x64, 0xFE);
	die(NULL);
}

#ifdef CONFIG_ACPICA

#include "src/acpi.h"

static __attribute__((noreturn)) void doShutdown(void) {
	AcpiEnterSleepStatePrep(5);
	smpCallFunction(die, NULL, false);
	asm("cli");
	AcpiEnterSleepState(5);
	die(NULL);
}

#else //CONFIG_ACPICA

static __attribute__((noreturn)) void doShutdown(void) {
	smpCallFunction(die, NULL, false);
	puts("It is now safe to turn off the system");
	fbPanicUpdate();
	die(NULL);
}

#endif //CONFIG_ACPICA

int sysPower(uint32_t magic, uint32_t mode) {
	struct Process *proc = getCurrentThread()->process;
	if (proc && proc->cred.euid) {
		return -EPERM;
	}
	if (magic != POWER_MAGIC || (mode != POWER_SHUTDOWN && mode != POWER_REBOOT)) {
		return -EINVAL;
	}
	printk("Powering off...\n");
	int error = kernelPrepareShutdown(mode);
	printk("Prepared for shutdown\n");
	if (error) return error;

	switch(mode) {
		case POWER_SHUTDOWN:
			doShutdown();
		case POWER_REBOOT:
			doReboot();
	}
	die(NULL);
}