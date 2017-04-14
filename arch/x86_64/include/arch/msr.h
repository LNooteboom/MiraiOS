#ifndef INCLUDE_ARCH_MSR_H
#define INCLUDE_ARCH_MSR_H

static inline void wrmsr(uint32_t addr, uint64_t value) {
	uint32_t high = value >> 32;
	asm volatile ("wrmsr" : : "c"(addr), "a"(value), "d"(high));
}

static inline uint64_t rdmsr(uint32_t addr) {
	uint32_t high;
	uint32_t low;
	asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(addr));
	return ((uint64_t)(high) << 32) + low;
}

#endif