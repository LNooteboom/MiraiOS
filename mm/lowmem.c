#include <global.h>

#include <mm/paging.h>
#include <mm/memset.h>
#include <spinlock.h>
#include <atomic.h>

#define BITS_PER_SEG	16

static uint16_t bitmap[LOWMEM_SIZE / PAGE_SIZE / BITS_PER_SEG];

physPage_t allocLowPhysPages(uint8_t nrofPages) {
	if (nrofPages > BITS_PER_SEG) {
		return NULL;
	}
	uint16_t bitmask = (1 << nrofPages) - 1;
	physPage_t ret = NULL;
	for (uint16_t curSeg = 0; curSeg < (LOWMEM_SIZE / PAGE_SIZE / BITS_PER_SEG); curSeg++) {
		uint16_t val = atomicXchg16(&(bitmap[curSeg]), 0);
		hexprintln(val);
		for (uint8_t i = 0; i <= BITS_PER_SEG - nrofPages; i++) {
			hexprintln(1 << i);
			if ((val & bitmask) == bitmask) {
				ret = (curSeg * BITS_PER_SEG) + i;
				bitmap[curSeg] = val | (bitmask << i);
				return ret;
			}
			val >>= 1;
		}
	}
	return NULL;
}

void deallocLowPhysPages(physPage_t page, uint16_t nrofPages) {
	uint16_t seg = page / BITS_PER_SEG;
	uint8_t bit = page % BITS_PER_SEG;
	uint16_t i = 0;
	while (i < nrofPages) {
		if (bit == 0 && nrofPages >= BITS_PER_SEG) {
			bitmap[seg] = 0;
			i += BITS_PER_SEG;
		} else {
			//bitmap[seg] &= ~(1 << bit);
			atomicAnd16(&(bitmap[seg]), ~(1 << bit));
			i++;
			bit++;
			if (bit == BITS_PER_SEG) {
				bit = 0;
				seg++;
			}
		}
	}
}