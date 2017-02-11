#include <global.h>

#include <mm/paging.h>
#include <mm/heap.h>
#include <spinlock.h>

#define LOWMEM_SIZE

static uint16_t *bitmap;

static spinlock_t bitmapLock;

static uintptr_t lowMemStart;
static uintptr_t lowMemEnd;

void mmLowInit(uintptr_t _lowMemStart, uintptr_t _lowMemEnd) {
	uintptr_t bitStart = _lowMemStart & ~(LOWMEM_SEPERATOR - 1);
	uintptr_t bitEnd;
	if (_lowMemEnd & (LOWMEM_SEPERATOR - 1)) {
		bitEnd = (_lowMemEnd & (LOWMEM_SEPERATOR - 1)) + LOWMEM_SEPERATOR;
	} else {
		bitEnd = _lowMemEnd;
	}
	bitmapLength = (bitEnd - bitStart) / LOWMEM_SEPERATOR;
	bitmap = vmalloc(bitmapLength * sizeof(uint16_t));
	uint8_t nrofBitsStart = (_lowMemStart - bitStart) / PAGE_SIZE;
	bitmap[0] = ~0 >> (16 - nrofBitsStart); //if nrofBitsStart == 3 then result = 0b00000111
	uint8_t nrofBitsEnd = (bitEnd - _lowMemEnd);
	bitmap[bitmapLength - 1] = ~0 << (16 - nrofBitsEnd); //if nrofBitsEnd == 3 then result = 0b11100000
}