#include "src/acpi.h"

#include <arch/acpi.h>
#include <mm/cache.h>
#include <mm/heap.h>
#include <mm/pagemap.h>
#include <mm/paging.h>
#include <mm/memset.h>
#include <sched/spinlock.h>
#include <sched/lock.h>
#include <sched/sleep.h>
#include <irq.h>
#include <io.h>
#include <arch/tlb.h>
#include <arch/map.h>
#include <print.h>

struct ThreadEntry {
	thread_t thread;
	struct ThreadEntry *next;
};

static struct ThreadEntry *tList = NULL;
static spinlock_t tListLock;

static void *tempio = NULL;
static physPage_t tempioPhys;
static spinlock_t tempioLock;

ACPI_STATUS AcpiOsInitialize(void) {
	return AE_OK;
}

ACPI_STATUS
AcpiOsTerminate(void) {
	return AE_OK;
}


/*
 * ACPI Table interfaces
 */
ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(void) {
	return acpiRsdpPhysAddr;
}

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *InitVal, ACPI_STRING *NewVal) {
	(void) InitVal;
	*NewVal = NULL;
	return AE_OK;
}

ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable) {
	(void) ExistingTable;
	*NewTable = NULL;
	return AE_OK;
}

ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32 *NewTableLength) {
	(void) ExistingTable;
	(void) NewTableLength;
	*NewAddress = 0;
	return AE_OK;
}


/*
 * Spinlock primitives
 */
ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle) {
	spinlock_t *hndl = kzalloc(sizeof(spinlock_t));
	if (!hndl) {
		return AE_NO_MEMORY;
	}
	*OutHandle = hndl;
	return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) {
	kfree(Handle);
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
	acquireSpinlock(Handle);
	return AE_OK;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {
	(void)Flags; //gets returned by AcpiOsAcquireLock
	releaseSpinlock(Handle);
}

/*
 * Semaphore primitives
 */
ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE *OutHandle) {
	(void) MaxUnits;
	semaphore_t *sem = kmalloc(sizeof(sem));
	if (!sem) {
		return AE_NO_MEMORY;
	}
	semInit(sem, InitialUnits);
	*OutHandle = sem;
	return AE_OK;
}

ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) {
	while (((semaphore_t *)Handle)->value <= 0) {
		semSignal(Handle);
	}
	kfree(Handle);
	return AE_OK;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout) {
	(void) Timeout; //TODO
	for (UINT32 i = 0; i < Units; i++) {
		semWait(Handle);
	}
	return AE_OK;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
	for (UINT32 i = 0; i < Units; i++) {
		semSignal(Handle);
	}
	return AE_OK;
}


/*
 * Mutex primitives. May be configured to use semaphores instead via
 * ACPI_MUTEX_TYPE (see platform/acenv.h)
 */
#if (ACPI_MUTEX_TYPE != ACPI_BINARY_SEMAPHORE)

ACPI_STATUS
AcpiOsCreateMutex (
    ACPI_MUTEX              *OutHandle);

void
AcpiOsDeleteMutex (
    ACPI_MUTEX              Handle);

ACPI_STATUS
AcpiOsAcquireMutex (
    ACPI_MUTEX              Handle,
    UINT16                  Timeout);

void
AcpiOsReleaseMutex (
    ACPI_MUTEX              Handle);

#endif


/*
 * Memory allocation and mapping
 */
void *AcpiOsAllocate(ACPI_SIZE Size) {
	return kmalloc(Size);
}

void *AcpiOsAllocateZeroed(ACPI_SIZE Size) {
	return kzalloc(Size);
}

void AcpiOsFree(void *Memory) {
	kfree(Memory);
}

void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Where, ACPI_SIZE Length) {
	void *ret = ioremap(Where, Length);
	return ret;
}

void AcpiOsUnmapMemory(void *LogicalAddress, ACPI_SIZE Size) {
	iounmap(LogicalAddress, Size);
}

ACPI_STATUS
AcpiOsGetPhysicalAddress(void *LogicalAddress, ACPI_PHYSICAL_ADDRESS *PhysicalAddress) {
	uintptr_t iaddr = (uintptr_t)LogicalAddress;
	uintptr_t remain = iaddr & (PAGE_SIZE - 1);
	iaddr -= remain;
	physPage_t page = mmGetPageEntry(iaddr);
	if (!page) {
		return AE_ERROR;
	}
	*PhysicalAddress = page + remain;
	return AE_OK;
}


/*
 * Memory/Object Cache
 */
ACPI_STATUS AcpiOsCreateCache(char *CacheName, UINT16 ObjectSize, UINT16 MaxDepth, ACPI_CACHE_T **ReturnCache) {
	(void) MaxDepth;
	struct Cache *c = kmalloc(sizeof(*c));
	if (!c) {
		return AE_NO_MEMORY;
	}
	cacheCreate(c, ObjectSize, CacheName);
	*ReturnCache = c;
	return AE_OK;
}

ACPI_STATUS AcpiOsDeleteCache(ACPI_CACHE_T *Cache) {
	cachePurge(Cache);
	kfree(Cache);
	return AE_OK;
}

ACPI_STATUS AcpiOsPurgeCache(ACPI_CACHE_T *Cache) {
	//cachePurge(Cache); Appears to be broken?
	printk("[ACPI] Ignoring cache purge\n");
	return AE_OK;
}

void *AcpiOsAcquireObject(ACPI_CACHE_T *Cache) {
	void *ret = cacheGet(Cache);
	if (!ret) return NULL;
	memset(ret, 0, Cache->objSize);
	return ret;
}

ACPI_STATUS AcpiOsReleaseObject (ACPI_CACHE_T *Cache, void *Object) {
	cacheRelease(Cache, Object);
	return AE_OK;
}


/*
 * Interrupt handlers
 */
ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine, void *Context) {
	void (*handler)(void *) = (void (*)(void *))ServiceRoutine;

	interrupt_t vec = allocIrqVec();
	int err = routeInterrupt(handler, Context, vec, 0, "ACPICA");
	if (err) return AE_ERROR;

	err = routeIrqLine(vec, InterruptNumber, HWIRQ_FLAG_ISA);
	if (err) return AE_ERROR;

	return AE_OK;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine) {
	(void) ServiceRoutine;
	unrouteInterrupt(InterruptNumber);
	return AE_OK;
}


/*
 * Threads and Scheduling
 */
ACPI_THREAD_ID AcpiOsGetThreadId(void) {
	return (UINT64)getCurrentThread();
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void *Context) {
	(void) Type;
	thread_t newThread;
	struct ThreadEntry *newEntry = kmalloc(sizeof(*newEntry));
	if (!newEntry) {
		return AE_ERROR;
	}
	if (kthreadCreate(&newThread, (void *(*)(void *))Function, Context, 0)) {
		return AE_ERROR;
	}
	newEntry->thread = newThread;
	acquireSpinlock(&tListLock);
	newEntry->next = tList;
	tList = newEntry;
	releaseSpinlock(&tListLock);
	return AE_OK;
}

void AcpiOsWaitEventsComplete(void) {
	struct ThreadEntry *en = tList;
	while (en) {
		kthreadJoin(en->thread, NULL);
		struct ThreadEntry *next = en->next;
		kfree(en);
		en = next;
	}
}

void AcpiOsSleep(UINT64 Milliseconds) {
	kthreadSleep(Milliseconds);
}

void
AcpiOsStall(UINT32 Microseconds) {
	//TODO
	if (Microseconds >= 1000) {
		kthreadSleep(Microseconds / 1000);
	} else {
		kthreadSleep(1);
	}
}


/*
 * Platform and hardware-independent I/O interfaces
 */
ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32 *Value, UINT32 Width) {
	switch (Width) {
		case 8:
			*Value = in8(Address);
			break;
		case 16:
			*Value = in16(Address);
			break;
		case 32:
			*Value = in32(Address);
			break;
		default:
			return AE_ERROR;
	}
	return AE_OK;
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width) {
	switch (Width) {
		case 8:
			out8(Address, Value);
			break;
		case 16:
			out16(Address, Value);
			break;
		case 32:
			out32(Address, Value);
			break;
	}
	return AE_OK;
}


/*
 * Platform and hardware-independent physical memory interfaces
 */
ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value, UINT32 Width) {
	acquireSpinlock(&tempioLock);
	physPage_t page = Address & ~(PAGE_SIZE - 1);
	if (!tempio) {
		tempio = ioremap(page, PAGE_SIZE);
	} else if (tempioPhys != page) {
		mmMapPage((uintptr_t)tempio, page, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_KERNEL); //todo set no cache
		tlbInvalidateGlobal(tempio, 1);
	}
	tempioPhys = page;

	uintptr_t addr = (uintptr_t)tempio + (Address - page);
	switch (Width) {
		case 8:
			*Value = *((uint8_t *)addr);
			break;
		case 16:
			*Value = *((uint16_t *)addr);
			break;
		case 32:
			*Value = *((uint32_t *)addr);
			break;
		case 64:
			*Value = *((uint64_t *)addr);
			break;
	}
	

	releaseSpinlock(&tempioLock);
	return AE_OK;
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width) {
	acquireSpinlock(&tempioLock);
	physPage_t page = Address & ~(PAGE_SIZE - 1);
	if (!tempio) {
		tempio = ioremap(page, PAGE_SIZE);
	} else if (tempioPhys != page) {
		mmMapPage((uintptr_t)tempio, page, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_KERNEL); //todo set no cache
		tlbInvalidateGlobal(tempio, 1);
	}
	tempioPhys = page;

	uintptr_t addr = (uintptr_t)tempio + (Address - page);
	switch (Width) {
		case 8:
			*((uint8_t *)addr) = Value;
			break;
		case 16:
			*((uint16_t *)addr) = Value;
			break;
		case 32:
			*((uint32_t *)addr) = Value;
			break;
		case 64:
			*((uint64_t *)addr) = Value;
			break;
	}
	

	releaseSpinlock(&tempioLock);
	return AE_OK;
}


/*
 * Platform and hardware-independent PCI configuration space access
 * Note: Can't use "Register" as a parameter, changed to "Reg" --
 * certain compilers complain.
 */
ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID *PciId, UINT32 Reg, UINT64 *Value, UINT32 Width) {
	//TODO
	return AE_SUPPORT;
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID *PciId, UINT32 Reg, UINT64 Value, UINT32 Width) {
	//TODO
	return AE_SUPPORT;
}


/*
 * Miscellaneous
 */
BOOLEAN
AcpiOsReadable(void *Pointer, ACPI_SIZE Length) {
	BOOLEAN ok = true;
	for (ACPI_SIZE i = 0; i < sizeToPages(Length); i++) {
		ok = mmGetPageEntry((uintptr_t)Pointer + i * PAGE_SIZE) != 0;
		if (!ok) break;
	}
	return ok;
}

BOOLEAN AcpiOsWritable(void *Pointer, ACPI_SIZE Length) {
	uint64_t ok;
	for (ACPI_SIZE i = 0; i < sizeToPages(Length); i++) {
		ok = mmGetPageEntry((uintptr_t)Pointer + i * PAGE_SIZE);
		if (!ok) break;
		ok = (*mmGetEntry((uintptr_t)Pointer + i * PAGE_SIZE, 0) & PAGE_FLAG_WRITE);
		if (!ok) break;
	}
	return ok != 0;
}

UINT64 AcpiOsGetTimer(void) {
	return jiffyCounter * (10000000/JIFFY_HZ);
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void *Info) {
	(void) Function;
	(void) Info;
	printk("ACPI Signal!\n");
	return AE_OK;
}

ACPI_STATUS AcpiOsEnterSleep(UINT8 SleepState, UINT32 RegaValue, UINT32 RegbValue) {
	//Sleep as in S-states, not kthreadSleep
	(void)SleepState;
	(void)RegaValue;
	(void)RegbValue;
	return AE_OK;
}


/*
 * Debug print routines
 */
void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf(const char *Format, ...) {
	va_list args;
	va_start(args, Format);
	vprintk(Format, args);
	va_end(args);
}

void AcpiOsVprintf(const char *Format, va_list Args) {
	vprintk(Format, Args);
}

void AcpiOsRedirectOutput(void *Destination) {
	(void) Destination;
	return;
}


/*
 * Debug IO
 */
/*ACPI_STATUS
AcpiOsGetLine (
    char                    *Buffer,
    UINT32                  BufferLength,
    UINT32                  *BytesRead);

ACPI_STATUS
AcpiOsInitializeDebugger (
    void);

void
AcpiOsTerminateDebugger (
    void);

ACPI_STATUS
AcpiOsWaitCommandReady (
    void);

ACPI_STATUS
AcpiOsNotifyCommandComplete (
    void);

void
AcpiOsTracePoint (
    ACPI_TRACE_EVENT_TYPE   Type,
    BOOLEAN                 Begin,
    UINT8                   *Aml,
    char                    *Pathname);
*/

/*
 * Obtain ACPI table(s)
 */
ACPI_STATUS AcpiOsGetTableByName(char *Signature, UINT32 Instance, ACPI_TABLE_HEADER **Table, ACPI_PHYSICAL_ADDRESS *Address) {
	return AE_SUPPORT;
}

ACPI_STATUS AcpiOsGetTableByIndex(UINT32 Index, ACPI_TABLE_HEADER **Table, UINT32 *Instance, ACPI_PHYSICAL_ADDRESS *Address) {
	return AE_SUPPORT;
}

ACPI_STATUS AcpiOsGetTableByAddress (ACPI_PHYSICAL_ADDRESS Address, ACPI_TABLE_HEADER **Table) {
	return AE_SUPPORT;
}


/*
 * Directory manipulation
 */
void *
AcpiOsOpenDirectory (
    char                    *Pathname,
    char                    *WildcardSpec,
    char                    RequestedFileType);

char *
AcpiOsGetNextFilename (
    void                    *DirHandle);

void
AcpiOsCloseDirectory (
    void                    *DirHandle);