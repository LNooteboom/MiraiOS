#include "efi.h"
#include <stdint.h>
#include <stddef.h>
#include <arch/bootinfo.h>
#include <mm/memset.h>

extern EFI_HANDLE imageHandle;
extern EFI_SYSTEM_TABLE *efiSystemTable;

//char initrdPath[] = "\\\0i\0n\0i\0t\0r\0d\0\0";
uint16_t initrdPath[] = L"initrd";
uint16_t initrdFail[] = L"Failed to load initrd!";

uint16_t tex[] = L"0";

EFI_GUID efiLoadedImageGuid = {
	.data1 = 0x5B1B31A1,
	.data2 = 0x9562,
	.data3 = 0x11d2,
    .data4 = {0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}
};

EFI_GUID efiLoadedImagePathGuid = {
	.data1 = 0xbc62157e,
	.data2 = 0x3e33,
	.data3 = 0x4fec,
	.data4 = {0x99,0x20,0x2d,0x3b,0x36,0xd7,0x50,0xdf}
};

EFI_GUID efiFileSystemGuid = {
	.data1 = 0x0964e5b22,
	.data2 = 0x6459,
	.data3 = 0x11d2,
	.data4 = {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}
};

EFI_GUID efiFileInfoGuid = {
	.data1 = 0x09576e92,
	.data2 = 0x6d3f,
	.data3 = 0x11d2,
	.data4 = {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}
};

#define devicePathNodeLength(a)     ( ((a)->Length[0]) | ((a)->Length[1] << 8) )
#define nextDevicePathNode(a)       ( (EFI_DEVICE_PATH_PROTOCOL *) ( ((uint8_t *) (a)) + devicePathNodeLength(a)))

static int64_t getKernelPath(EFI_DEVICE_PATH_PROTOCOL *prot, uint16_t **kernelPath) {
	EFI_DEVICE_PATH_PROTOCOL *prot2 = prot;
	size_t pSize = 0;
	while (prot2->Type != 0x7F || prot2->SubType != 0xFF) {
		if (prot2->Type == 0x04 && prot2->SubType == 0x04) {
			pSize += devicePathNodeLength(prot2) - sizeof(EFI_DEVICE_PATH_PROTOCOL) + 2;
		}
		prot2 = nextDevicePathNode(prot2);
	}
	//allocate buffer
	pSize += 2; //add space for beginning backslash
	uint16_t *buf;
	if (efiCall3(efiSystemTable->BootServices->AllocatePool, 2, pSize, (uint64_t)&buf))
		return -1;

	uint64_t size = 0;
	prot2 = prot;
	int bufIndex = 0;
	while (prot2->Type != 0x7F || prot2->SubType != 0xFF) {
		if (prot2->Type == 0x04 && prot2->SubType == 0x04) {
			int curSize = devicePathNodeLength(prot2) - sizeof(EFI_DEVICE_PATH_PROTOCOL) - 2;
			uint16_t *str = (uint16_t*)(prot2) + 2;
			if (str[0] != L'\\') {
				buf[bufIndex++] = L'\\';
				size += 2;
			}
			for (int i = 0; i < curSize / 2; i++) {
				buf[bufIndex++] = str[i];
			}
			size += curSize;
		}
		prot2 = nextDevicePathNode(prot2);
	}
	buf[bufIndex] = 0;
	*kernelPath = buf;

	return size;
}

int efiHandleInitrd(void) {
	EFI_LOADED_IMAGE_PROTOCOL *loadedImageProt;
	if (efiCall3(efiSystemTable->BootServices->HandleProtocol, (uint64_t)imageHandle, (uint64_t)&efiLoadedImageGuid, (uint64_t)&loadedImageProt))
		goto error;
	//get filesystem protocol from device handle
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fsProt;
	if (efiCall3(efiSystemTable->BootServices->HandleProtocol, (uint64_t)loadedImageProt->DeviceHandle, (uint64_t)&efiFileSystemGuid, (uint64_t)&fsProt))
		goto error;

	//open volume
	EFI_FILE_PROTOCOL *rootProt;
	if (efiCall2(fsProt->OpenVolume, (uint64_t)fsProt, (uint64_t)&rootProt))
		goto error;

	//make filename relative to kernel location
	EFI_DEVICE_PATH_PROTOCOL *kernelPathProt;
	if (efiCall3(efiSystemTable->BootServices->HandleProtocol, (uint64_t)imageHandle, (uint64_t)&efiLoadedImagePathGuid, (uint64_t)&kernelPathProt))
		goto error;
	int64_t kernelPathSize;
	uint16_t *kernelPath;
	if ((kernelPathSize = getKernelPath(kernelPathProt, &kernelPath)) < 0)
		goto error;

	//find last backslash in kernelPath
	for (int i = (kernelPathSize / 2) - 1; i >= 0; i--) {
		if (kernelPath[i] == L'\\') {
			kernelPathSize = (i + 1) * 2;
			break;
		}
	}
	size_t initrdPathSize = kernelPathSize + sizeof(initrdPath);
	uint16_t *newInitrdPath;
	if (efiCall3(efiSystemTable->BootServices->AllocatePool, 2, initrdPathSize, (uint64_t)&newInitrdPath))
		goto error;
	memcpy(newInitrdPath, kernelPath, kernelPathSize);
	//copy const initrd
	memcpy(&newInitrdPath[kernelPathSize / 2], initrdPath, sizeof(initrdPath));

	//deallocate kernel name buffer
	efiCall1(efiSystemTable->BootServices->FreePool, (uint64_t)kernelPath);

	//open file
	EFI_FILE_PROTOCOL *initrdProt;
	if (efiCall5(rootProt->Open, (uint64_t)rootProt, (uint64_t)&initrdProt, (uint64_t)newInitrdPath, 1, 0))
		goto error;
	//deallocate path buffer
	efiCall1(efiSystemTable->BootServices->FreePool, (uint64_t)newInitrdPath);

	//get size
	//allocate buffer
	uint64_t initrdInfoSize = sizeof(EFI_FILE_INFO) + sizeof(initrdPath);
	EFI_FILE_INFO *initrdInfo;
	if (efiCall3(efiSystemTable->BootServices->AllocatePool, 2, initrdInfoSize, (uint64_t)&initrdInfo))
		goto error;
	if (efiCall4(initrdProt->GetInfo, (uint64_t)initrdProt, (uint64_t)&efiFileInfoGuid, (uint64_t)&initrdInfoSize, (uint64_t)initrdInfo))
		goto error;
	size_t initrdSize = initrdInfo->FileSize;
	//free buffer
	efiCall1(efiSystemTable->BootServices->FreePool, (uint64_t)initrdInfo);

	//allocate pages for initrd
	uint64_t initrdNrofPages = initrdSize / 0x1000;
	if (initrdSize & 0xFFF) {
		initrdNrofPages++;
	}
	void *initrd;
	if (efiCall4(efiSystemTable->BootServices->AllocatePages, 0, 2, initrdNrofPages, (uint64_t)&initrd))
		goto error;

	efiCall2(initrdProt->SetPosition, (uint64_t)initrdProt, 0);
	size_t sz = initrdSize;
	//read initrd into memory
	if (efiCall3(initrdProt->Read, (uint64_t)initrdProt, (uint64_t)&initrdSize, (uint64_t)initrd))
		goto error;

	hexprint2(initrdSize, efiSystemTable);
	//close initrd file
	efiCall1(initrdProt->Close, (uint64_t)initrdProt);
	
	//close root file
	efiCall1(rootProt->Close, (uint64_t)rootProt);

	bootInfo.initrd = initrd;
	bootInfo.initrdLen = initrdSize;
	//while (1);
	return 0;

	error:
	efiCall2(efiSystemTable->ConOut->OutputString, (uint64_t)efiSystemTable->ConOut, (uint64_t)initrdFail);
	return -1;
}