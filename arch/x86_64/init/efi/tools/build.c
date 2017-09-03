#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define NROF_SECTIONS 3
#define VMEM_OFFSET 0xFFFFFFFF80000000

struct PeSect {
	char name[8];
	uint32_t virtualSize;
	uint32_t virtualAddress;
	uint32_t rawSize;
	uint32_t rawPointer;

	uint32_t ptrToRelocs;
	uint32_t ptrToLineNums;
	uint16_t nrofRelocs;
	uint16_t nrofLineNums;
	uint32_t characteristics;
};

struct PeHeader {
	//DOS header
	uint16_t mzSig;
	uint16_t dosHdr[29];
	uint32_t peOffset;

	//PE header
	char peSig[4];
	uint16_t machine;
	uint16_t nrofSections;
	uint32_t timestamp;
	uint32_t symtab;
	uint32_t nrofSyms;
	uint16_t optHeaderSize;
	uint16_t characteristics;

	//(Not-so) Optional header
	uint16_t magic;
	uint8_t majorLinkerVersion;
	uint8_t minorLinkerVersion;
	uint32_t sizeOfCode;
	uint32_t sizeOfInitializedData;
	uint32_t sizeOfUninitializedData;
	uint32_t entryPoint;
	uint32_t baseOfText;
	uint64_t imageBase;
	uint32_t sectionAlignment;
	uint32_t fileAlignment;

	uint16_t majorOSVersion;
	uint16_t minorOSVersion;
	uint16_t majorImageVersion;
	uint16_t minorImageVersion;
	uint16_t majorSubsystemVersion;
	uint16_t minorSubsystemVersion;

	uint32_t reserved;
	uint32_t sizeOfImage;
	uint32_t sizeOfHeaders;
	uint32_t checksum;
	uint16_t subsystem;
	uint16_t dllCharacteristics;

	uint64_t sizeOfStackReserve;
	uint64_t sizeOfStackCommit;
	uint64_t sizeOfHeapReserve;
	uint64_t sizeOfHeapCommit;

	uint32_t loaderFlags;
	uint32_t numberOfRvaAndSizes;

	uint64_t rvaAndSizes[6];

	struct PeSect sects[NROF_SECTIONS];
} __attribute__((packed));

struct ElfHeader {
	char magic[4];
	uint8_t bits;
	uint8_t endianness;
	uint8_t abiVer;
	uint8_t osABI;
	uint64_t unused;

	uint16_t type;
	uint16_t machine;
	uint32_t elfVer;
	uint64_t entryAddr;
	uint64_t phOff;
	uint64_t shOff;
	uint32_t flags;
	uint16_t ehSize;

	uint16_t phentSize;
	uint16_t phnum;
	uint16_t shentSize;
	uint16_t shnum;

	uint16_t shstrndx;
};

struct ElfPHEntry {
	uint32_t type;
	uint32_t flags;
	uint64_t pOffset;
	uint64_t pVAddr;
	uint64_t undefined;
	uint64_t pFileSz;
	uint64_t pMemSz;
	uint64_t alignment;
};

static struct PeHeader peHdr = {
	.mzSig = 0x5a4d,
	.peOffset = 0x40,
	.peSig = "PE",
	.machine = 0x8664,
	.nrofSections = NROF_SECTIONS,
	.optHeaderSize = 160,
	.characteristics = 0x2226,
	.magic = 0x20b,
	.majorLinkerVersion = 1,

	.imageBase = 0x10000000,
	.sectionAlignment = 0x1000,
	.fileAlignment = 0x1000,
	.majorOSVersion = 4,
	.majorSubsystemVersion = 5,
	.minorSubsystemVersion = 2,
	.sizeOfHeaders = 0x1000,
	.subsystem = 10, //EFI application
	.dllCharacteristics = 0x160,

	.sizeOfStackReserve = 0x200000,
	.sizeOfStackCommit = 0x1000,
	.sizeOfHeapReserve = 0x100000,
	.sizeOfHeapCommit = 0x1000,

	.numberOfRvaAndSizes = 6
};

char zeroBuf[0x1000];

static struct ElfHeader kernelElfHeader;

static int curPESection = 0;
static uint64_t imageEnd = 0;

static void usage(void) {
	fprintf(stderr, "Usage: build [Kernel ELF] [Output file]\n");
}

static int checkElfHeader(void) {
	if (memcmp(&kernelElfHeader.magic, "\x7f""ELF", 4)) {
		fprintf(stderr, "Error: Wrong magic bytes\n");
		return 1;
	}
	if (kernelElfHeader.type != 2) {
		fprintf(stderr, "Error: Kernel is not executable\n");
		return 1;
	}
	if (kernelElfHeader.machine != 0x3E) {
		fprintf(stderr, "Error: Kernel arch is not x86_64\n");
		return 1;
	}
	if (kernelElfHeader.phentSize != sizeof(struct ElfPHEntry)) {
		fprintf(stderr, "Error: Program header size is not 56 bytes\n");
		return 1;
	}
	return 0;
}

static size_t getKernelSize(FILE *kernel) {
	struct stat buf;
	if (fstat(fileno(kernel), &buf)) {
		return 0;
	}
	return buf.st_size;
}

static uint64_t align(uint64_t val) {
	if (val & 0xFFF) {
		val &= ~0xFFF;
		val += 0x1000;
	}
	return val;
}

static void parsePHEnt(struct ElfPHEntry *entry) {
	if (entry->type != 1) {
		return;
	}
	if (curPESection >= NROF_SECTIONS) {
		fprintf(stderr, "Error: Too many kernel sections\n");
		return;
	}
	entry->pVAddr += 0x1000; //increment because virtual addresses cant be zero
	if (entry->flags & 1) {
		if (entry->pVAddr < VMEM_OFFSET) {
			strcpy(peHdr.sects[curPESection].name, ".setup");
			peHdr.baseOfText = entry->pVAddr;
		} else { //.setup section
			entry->pVAddr -= VMEM_OFFSET + 0x01000000;
			strcpy(peHdr.sects[curPESection].name, ".text");
		}
		
		peHdr.sects[curPESection].virtualSize = entry->pFileSz;
		peHdr.sects[curPESection].virtualAddress = entry->pVAddr;
		peHdr.sects[curPESection].rawSize = align(entry->pFileSz);
		peHdr.sects[curPESection].rawPointer = align(entry->pOffset) + 0x1000; //add pe header size to offset
		peHdr.sects[curPESection].characteristics = 0x60000020;
		peHdr.sizeOfCode += align(entry->pFileSz);
		curPESection++;
		return;
	}
	if (entry->flags & 2) { //Has write permission
		/*entry->pVAddr -= VMEM_OFFSET + 0x01000000;
		strcpy(peHdr.sects[curPESection].name, ".data");
		peHdr.sects[curPESection].virtualSize = entry->pFileSz;
		peHdr.sects[curPESection].virtualAddress = entry->pVAddr;
		peHdr.sects[curPESection].rawSize = align(entry->pFileSz);
		peHdr.sects[curPESection].rawPointer = align(entry->pOffset) + 0x1000; //add pe header size to offset
		peHdr.sects[curPESection].characteristics = 0xc0000040;
		peHdr.sizeOfInitializedData += align(entry->pFileSz);
		curPESection++;

		if (entry->pFileSz >= entry->pMemSz)
			return;

		strcpy(peHdr.sects[curPESection].name, ".bss");
		peHdr.sects[curPESection].virtualSize = align(entry->pMemSz - entry->pFileSz);
		peHdr.sects[curPESection].virtualAddress = align(entry->pVAddr + entry->pFileSz);
		peHdr.sects[curPESection].rawSize = align(entry->pMemSz - entry->pFileSz);
		//peHdr.sects[curPESection].rawSize = 0;
		//peHdr.sects[curPESection].rawPointer = align(entry->pOffset + entry->pFileSz) + 0x1000;
		peHdr.sects[curPESection].characteristics = 0xc0000080;
		peHdr.sizeOfUninitializedData += align(entry->pMemSz - entry->pFileSz);*/

		entry->pVAddr -= VMEM_OFFSET + 0x01000000;
		strcpy(peHdr.sects[curPESection].name, ".data");
		peHdr.sects[curPESection].virtualSize = entry->pFileSz;
		peHdr.sects[curPESection].virtualAddress = entry->pVAddr;
		peHdr.sects[curPESection].rawSize = align(entry->pFileSz);
		peHdr.sects[curPESection].rawPointer = align(entry->pOffset) + 0x1000; //add pe header size to offset
		peHdr.sects[curPESection].characteristics = 0xc0000040;
		peHdr.sizeOfInitializedData += align(entry->pFileSz);
		curPESection++;

		uint64_t end = align(entry->pFileSz) + entry->pVAddr + 0x1000;
		if (end > imageEnd)
			imageEnd = end;
		curPESection++;
		return;
	}
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		usage();
		return 1;
	}

	//open kernel elf
	FILE *kernel = fopen(argv[1], "r");
	if (!kernel) {
		fprintf(stderr, "Failed to open %s: %m\n", argv[1]);
		return 1;
	}
	size_t kernelSize = getKernelSize(kernel);
	if (!kernelSize) {
		fprintf(stderr, "Failed to stat kernel\n");
		return 1;
	}
	size_t read = fread(&kernelElfHeader, 1, sizeof(struct ElfHeader), kernel);
	if (read != sizeof(struct ElfHeader)) {
		fprintf(stderr, "Error: reading kernel ELF header\n");
		return 1;
	}

	if (checkElfHeader()) {
		return 1;
	}
	//Get program header
	if (fseek(kernel, kernelElfHeader.phOff, SEEK_SET)) {
		fprintf(stderr, "Error: Kernel seek failed\n");
		return 1;
	}
	size_t phSize = sizeof(struct ElfPHEntry) * kernelElfHeader.phnum;
	struct ElfPHEntry *kernelPHs = malloc(phSize);
	if (!kernelPHs) {
		fprintf(stderr, "Error: Out of memory\n");
		return 1;
	}
	if (fread(kernelPHs, 1, phSize, kernel) != phSize) {
		fprintf(stderr, "Error: Program header read failed\n");
		return 1;
	}

	//parse program headers into pe sections
	for (int i = 0; i < kernelElfHeader.phnum; i++) {
		parsePHEnt(&kernelPHs[i]);
	}
	free(kernelPHs);
	//fill in pe header
	peHdr.entryPoint = kernelElfHeader.entryAddr + 0x1000;
	peHdr.sizeOfImage = imageEnd;

	//Create PE file
	FILE *dest = fopen(argv[2], "w");
	if (!dest) {
		fprintf(stderr, "Failed to open %s: %m\n", argv[2]);
		return 1;
	}
	//write PE header
	if (fwrite(&peHdr, 1, sizeof(struct PeHeader), dest) != sizeof(struct PeHeader)) {
		fprintf(stderr, "Failed to write PE header\n");
		return 1;
	}
	//write header padding
	if (fwrite(&peHdr, 1, 0x1000 - sizeof(struct PeHeader), dest) != 0x1000 - sizeof(struct PeHeader)) {
		fprintf(stderr, "Failed to write padding\n");
		return 1;
	}

	//now append kernel to dest
	rewind(kernel);
	int c;
	while ((c = fgetc(kernel)) != EOF) {
		if (putc(c, dest) == EOF) {
			fprintf(stderr, "Failed to copy kernel to dest");
			return 1;
		}
	}

	if (fclose(dest)) {
		fprintf(stderr, "Failed to close dest\n");
		return 1;
	}
	fclose(kernel);
	printf("Done\n");
	return 0;
}