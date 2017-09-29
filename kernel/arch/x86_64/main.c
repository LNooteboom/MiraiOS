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
#include <arch/idt.h>

#include <arch/msr.h>

unsigned int nrofCPUs = 0;
unsigned int nrofActiveCPUs = 1;
struct CpuInfo *cpuInfos = NULL;

bool perCpuTimer = false;

extern void jiffyIrq(void);
extern void reschedIPI(void);
extern void syscallInit(void);

void earlyArchInit(void) {
	uint64_t pat = rdmsr(0x277);
	pat &= ~(0xFF << 8);
	pat |= (0x01 << 8);
	wrmsr(0x277, pat);
	acpiInit();

	lapicInit();
	ioApicInit();

	tlbInit();
	smpCallInit();

	syscallInit();
}

void archInit(void) {
	mapIdtEntry(jiffyIrq, JIFFY_VEC, 0);
	mapIdtEntry(reschedIPI, RESCHED_VEC, 0);
	if (perCpuTimer) {
		//use lapic timer
		printk("[x86] Using LAPIC timer for jiffy clocksource\n");
		lapicEnableTimer(JIFFY_VEC);
	} else {
		//use PIT
		printk("[x86] Using PIT for jiffy clocksource\n");
		routeIrqLine(JIFFY_VEC, 0, HWIRQ_FLAG_ISA);
		i8253SetFreq(JIFFY_HZ);
		i8253State(true);
	}

	lapicDoSMPBoot();
}

void archFini(void) {

}