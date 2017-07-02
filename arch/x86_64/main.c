#include <arch.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <arch/acpi.h>
#include <arch/cpu.h>
#include <arch/ioapic.h>
#include <arch/tlb.h>
#include <sched/smpcall.h>
#include <drivers/timer/i8253.h>

#define JIFFY_VEC	0xC2
#define RESCHED_VEC	0xC3

unsigned int nrofCPUs = 0;
unsigned int nrofActiveCPUs = 1;
struct cpuInfo *cpuInfos = NULL;

bool perCpuTimer = false;

extern void jiffyIrq(void);
extern void reschedIPI(void);

void earlyArchInit(void) {
	acpiInit();
	lapicInit();
	ioApicInit();
	tlbInit();
	smpCallInit();
}

void archInit(void) {
	routeInterrupt(jiffyIrq, JIFFY_VEC, 0);
	routeInterrupt(reschedIPI, RESCHED_VEC, 0);
	if (perCpuTimer) {
		//use lapic timer
		lapicEnableTimer(JIFFY_VEC);
	} else {
		//use PIT
		routeIrqLine(JIFFY_VEC, 0, IRQ_FLAG_ISA);
		i8253SetFreq(JIFFY_HZ);
		i8253State(true);
	}

	lapicDoSMPBoot();
}

void archFini(void) {

}