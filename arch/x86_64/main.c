#include <arch.h>

#include <stdint.h>
#include <acpi.h>
#include <apic.h>
#include <ioapic.h>

void archInit(void) {
	acpiInit();
	lapicInit();
	ioApicInit();
}

void archFini(void) {

}