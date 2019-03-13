#include "src/acpi.h"

#include <modules.h>
#include <print.h>

int acpicaInitialize(void) {
	printk("Starting ACPI...\n");
	ACPI_STATUS err = AE_OK;

	err = AcpiInitializeSubsystem();
	if (ACPI_FAILURE(err)) goto out;

	err = AcpiInitializeTables(NULL, 16, FALSE);
	if (ACPI_FAILURE(err)) goto out;

	err = AcpiLoadTables();
	if (ACPI_FAILURE(err)) goto out;

	err = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(err)) goto out;

	err = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(err)) goto out;

	/*AcpiEnterSleepStatePrep(5);
	asm("cli");
	AcpiEnterSleepState(5);*/

	out:
	return (ACPI_FAILURE(err))? 0 : -err;
}
MODULE_INIT_LEVEL(acpicaInitialize, 1);