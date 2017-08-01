#ifndef EFI_H
#define EFI_H

#include <stdint.h>

typedef void *EFI_FUNC;

typedef int64_t EFI_STATUS;

int64_t efiCall0(EFI_FUNC func);
int64_t efiCall1(EFI_FUNC func, uint64_t a1);
int64_t efiCall2(EFI_FUNC func, uint64_t a1, uint64_t a2);
int64_t efiCall3(EFI_FUNC func, uint64_t a1, uint64_t a2, uint64_t a3);
int64_t efiCall4(EFI_FUNC func, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4);
int64_t efiCall5(EFI_FUNC func, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5);
int64_t efiCall6(EFI_FUNC func, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6);

typedef void *EFI_HANDLE;

typedef struct {
	uint64_t Signature;
	uint32_t Revision;
	uint32_t HeaderSize;
	uint32_t CRC32;
	uint32_t Reserved;
} EFI_TABLE_HEADER;

typedef struct {
	EFI_FUNC Reset;
	EFI_FUNC OutputString;
	EFI_FUNC TestString;
	EFI_FUNC QueryMode;
	EFI_FUNC SetMode;
	EFI_FUNC SetAttribute;
	EFI_FUNC ClearScreen;
	EFI_FUNC SetCursorPosition;
	EFI_FUNC EnableCursor;
	void *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
	uint32_t type;
	uint64_t physicalStart;
	uint64_t virtualStart;
	uint64_t nrofPages;
	uint64_t attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef struct {
	EFI_TABLE_HEADER Hdr;
	EFI_FUNC RaiseTPL;
	EFI_FUNC RestoreTPL;
	EFI_FUNC AllocatePages;
	EFI_FUNC FreePages;
	EFI_FUNC GetMemoryMap;
	EFI_FUNC AllocatePool;
	EFI_FUNC FreePool;
	EFI_FUNC CreateEvent;
	EFI_FUNC SetTimer;
	EFI_FUNC WaitForEvent;
	EFI_FUNC SignalEvent;
	EFI_FUNC CloseEvent;
	EFI_FUNC CheckEvent;
	EFI_FUNC InstalProtocolInterface;
	EFI_FUNC ReinstallProtocolInterface;
	EFI_FUNC UninstallProtocolInterface;
	EFI_FUNC HandleProtocol;
	void *Reserved;
	EFI_FUNC RegisterProtocolNotify;
	EFI_FUNC LocateHandle;
	EFI_FUNC LocateDevicePath;
	EFI_FUNC InstallConfigurationTable;
	EFI_FUNC LoadImage;
	EFI_FUNC StartImage;
	EFI_FUNC Exit;
	EFI_FUNC UnloadImage;
	EFI_FUNC ExitBootServices;
	EFI_FUNC GetNextMonotonicCount;
	EFI_FUNC Stall;
	EFI_FUNC SetWatchdogTimer;
	EFI_FUNC ConnectController;
	EFI_FUNC DisconnectController;
	EFI_FUNC OpenProtocol;
	EFI_FUNC CloseProtocol;
	EFI_FUNC OpenProtocolInformation;
	EFI_FUNC ProtocolsPerHandle;
	EFI_FUNC LocateHandleBuffer;
	EFI_FUNC LocateProtocol;
	EFI_FUNC InstallMultipleProtocolInterfaces;
	EFI_FUNC UninstallMultipleProtocolInterfaces;
	EFI_FUNC CalculateCRC32;
	EFI_FUNC CopyMem;
	EFI_FUNC SetMem;
	EFI_FUNC CreateEventEx;
} EFI_BOOT_SERVICES;

typedef struct {
	EFI_TABLE_HEADER Hdr;
	void *FirmwareVendor;
	uint32_t FirmwareRevision;

	EFI_HANDLE ConsoleInHandle;
	void *ConIn;
	EFI_HANDLE ConsoleOutHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
	EFI_HANDLE StandardErrorHandle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;

	void *RuntimeServices;
	EFI_BOOT_SERVICES *BootServices;

	uint64_t NumberOfTableEntries;
	void *ConfigurationTable;
} EFI_SYSTEM_TABLE;

typedef struct {
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];
} EFI_GUID;


typedef enum {
	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask,
	PixelBltOnly,
	PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
	uint32_t RedMask;
	uint32_t GreenMask;
	uint32_t BlueMask;
	uint32_t ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct {
	uint32_t Version;
	uint32_t HorizontalResolution;
	uint32_t VerticalResolution;
	EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
	EFI_PIXEL_BITMASK PixelInformation;
	uint32_t PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
	uint32_t MaxMode;
	uint32_t Mode;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
	uint64_t SizeOfInfo;
	uint64_t FrameBufferBase;
	uint64_t FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct {
	EFI_FUNC QueryMode;
	EFI_FUNC SetMode;
	EFI_FUNC Blt;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} EFI_GRAPHICS_OUTPUT_PROTCOL;

#endif