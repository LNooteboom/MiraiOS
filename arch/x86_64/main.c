#include <arch.h>

#include <stdint.h>
#include <stddef.h>
#include <arch/acpi.h>
#include <arch/cpu.h>
#include <arch/ioapic.h>

unsigned int nrofCPUs = 0;
struct cpuInfo *cpuInfos = NULL;

void archInit(void) {
	acpiInit();
	lapicInit();
	ioApicInit();
}

void archFini(void) {

}