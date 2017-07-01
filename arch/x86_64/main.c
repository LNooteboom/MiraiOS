#include <arch.h>

#include <stdint.h>
#include <stddef.h>
#include <arch/acpi.h>
#include <arch/cpu.h>
#include <arch/ioapic.h>
#include <arch/tlb.h>
#include <sched/smpcall.h>

unsigned int nrofCPUs = 0;
unsigned int nrofActiveCPUs = 1;
struct cpuInfo *cpuInfos = NULL;

void archInit(void) {
	acpiInit();
	lapicInit();
	ioApicInit();
	tlbInit();
	smpCallInit();
}

void archFini(void) {

}