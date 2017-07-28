#include "efi.h"
#include <stdint.h>

void *testString = ("H\0e\0l\0l\0o\0 \0f\0r\0o\0m\0 \0C\0!\0");

void efiMain(EFI_HANDLE imageHandler, EFI_SYSTEM_TABLE *efiSystemTable) {
	efiCall2(efiSystemTable->ConOut->OutputString, (uint64_t)efiSystemTable->ConOut, (uint64_t)testString);
	while (1);
}