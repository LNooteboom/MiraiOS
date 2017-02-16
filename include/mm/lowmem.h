#ifndef INCLUDE_LOWMEM_H
#define INCLUDE_LOWMEM_H

#define LOWMEM_SIZE			0x01000000

physPage_t allocLowPhysPages(uint8_t nrofPages);

void deallocLowPhysPages(physPage_t page, uint16_t nrofPages)

#endif