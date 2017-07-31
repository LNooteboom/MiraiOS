#include "efi.h"
#include <stdint.h>
#include <stdbool.h>
#include <mm/paging.h>
#include <arch/bootinfo.h>

void *testString = "H\0e\0l\0l\0o\0 \0f\0r\0o\0m\0 \0C\0!\0\0";
char *errorString = "E\0F\0I\0 \0e\0r\0r\0o\0r\0 \0o\0c\0c\0u\0r\0e\0d\0!\0\0";

struct bootInfo bootInfo;

EFI_HANDLE imageHandle;
EFI_SYSTEM_TABLE *efiSystemTable;

EFI_GUID efiGopGuid = {
	.data1 = 0x9042A9DE,
	.data2 = 0x23DC,
	.data3 = 0x4A38,
	.data4 = {0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A}
};

static int efiHandleMmap(uint64_t *mmapKey) {
	//Get mmap
	char *mmapBuf;
	uint64_t mmapSize = PAGE_SIZE;
	//uint64_t mmapKey;
	uint64_t mmapDescSize;
	uint32_t mmapDescVer;
	while (true) {
		if (efiCall4(efiSystemTable->BootServices->AllocatePages, 0, 2, mmapSize / PAGE_SIZE, (uint64_t)&mmapBuf))
			goto error;
		EFI_STATUS ret = efiCall5(efiSystemTable->BootServices->GetMemoryMap, (uint64_t)&mmapSize, (uint64_t)mmapBuf, (uint64_t)mmapKey, (uint64_t)&mmapDescSize, (uint64_t)&mmapDescVer);
		if (!ret) {
			break;
		}
		if (ret == 5) {
			//Buffer too small
			efiCall2(efiSystemTable->BootServices->FreePages, (uint64_t)mmapBuf, mmapSize / PAGE_SIZE);
			mmapSize += PAGE_SIZE;
			continue;
		}
		goto error;
	}

	//translate mmap
	uint64_t mmapLen = mmapSize / mmapDescSize;
	struct mmapEntry *newMmap = (struct mmapEntry *)mmapBuf;
	for (uint64_t i = 0; i < mmapLen; i++) {
		EFI_MEMORY_DESCRIPTOR *oldMmap = (EFI_MEMORY_DESCRIPTOR *)&mmapBuf[i * mmapDescSize];
		physPage_t addr = oldMmap->physicalStart;
		uint64_t nrofPages = oldMmap->nrofPages;
		uint32_t attr;
		switch (oldMmap->type) {
			case 3: //efiBootServicesCode
			case 4: //efiBootServicesData
			case 7: //efiConventionalMemory
				attr = 1; //type = usable
				break;
			case 9: //efiACPIReclaimMemory
				attr = 3; //type = reclaim
				break;
			default:
				attr = 2; //type = unused
				break;
		}
		newMmap[i].addr = addr;
		newMmap[i].nrofPages = nrofPages;
		newMmap[i].attr = attr;
	}
	bootInfo.mmap = newMmap;
	bootInfo.mmapLen = mmapLen;
	bootInfo.lowMemReservedEnd = (void*)0x1000; //only reserve BDA page


	return 0;

	error:
	efiCall2(efiSystemTable->ConOut->OutputString, (uint64_t)efiSystemTable->ConOut, (uint64_t)errorString);
	return -1;
}

static inline void efiGetShiftAndSize(uint32_t mask, uint8_t *shift, uint8_t *size) {
	bool on = false;
	for (int i = 0; i < 32; i++) {
		char bit = (mask >> i) & 1;
		if (!on && bit) {
			on = true;
			*shift = i;
			continue;
		} else if (on && !bit) {
			*size = i - *shift;
			return;
		}
	}
}

static int efiHandleGop(void) {
	EFI_GRAPHICS_OUTPUT_PROTCOL *prot;
	if (efiCall3(efiSystemTable->BootServices->LocateProtocol, (uint64_t)&efiGopGuid, 0, (uint64_t)&prot))
		goto error;

	bootInfo.fbAddr = prot->Mode->FrameBufferBase;
	bootInfo.fbSize = prot->Mode->FrameBufferSize;
	bootInfo.fbXRes = prot->Mode->Info->HorizontalResolution;
	bootInfo.fbYres = prot->Mode->Info->VerticalResolution;
	bootInfo.fbBpp = 4;
	bootInfo.fbPitch = prot->Mode->Info->PixelsPerScanLine / 4;
	switch (prot->Mode->Info->PixelFormat) {
		case PixelRedGreenBlueReserved8BitPerColor:
			bootInfo.fbIsRgb = false;
			bootInfo.fbRSize = 8;
			bootInfo.fbRShift = 0;
			bootInfo.fbGSize = 8;
			bootInfo.fbGShift = 8;
			bootInfo.fbBSize = 8;
			bootInfo.fbBShift = 16;
			bootInfo.fbResSize = 8;
			bootInfo.fbResShift = 24;
			break;
		case PixelBlueGreenRedReserved8BitPerColor:
			bootInfo.fbIsRgb = true;
			break;
		case PixelBitMask:
			efiGetShiftAndSize(prot->Mode->Info->PixelInformation.RedMask, &bootInfo.fbRShift, &bootInfo.fbRSize);
			efiGetShiftAndSize(prot->Mode->Info->PixelInformation.GreenMask, &bootInfo.fbGShift, &bootInfo.fbGSize);
			efiGetShiftAndSize(prot->Mode->Info->PixelInformation.BlueMask, &bootInfo.fbBShift, &bootInfo.fbBSize);
			efiGetShiftAndSize(prot->Mode->Info->PixelInformation.ReservedMask, &bootInfo.fbResShift, &bootInfo.fbResSize);
			break;
		default:
			goto error;
	}
	
	return 0;

	error:
	efiCall2(efiSystemTable->ConOut->OutputString, (uint64_t)efiSystemTable->ConOut, (uint64_t)errorString);
	return -1;
}

void efiMain(EFI_HANDLE _imageHandle, EFI_SYSTEM_TABLE *_efiSystemTable) {
	imageHandle = _imageHandle;
	efiSystemTable = _efiSystemTable;

	if (efiHandleGop())
		goto error;

	uint64_t mmapKey;
	if (efiHandleMmap(&mmapKey))
		goto error;

	efiCall2(efiSystemTable->ConOut->OutputString, (uint64_t)efiSystemTable->ConOut, (uint64_t)testString);

	//exit boot services
	efiCall2(efiSystemTable->BootServices->ExitBootServices, (uint64_t)imageHandle, mmapKey);
	efiFini();
	kmain();

	error:
	while (true) {
		asm ("hlt");
	}
}