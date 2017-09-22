#ifndef INCLUDE_ATOMIC_H
#define INCLUDE_ATOMIC_H

/*
Exchanges two 16-bit values atomically
*/
static inline uint16_t atomicXchg16(uint16_t *ptr, uint16_t value) {
	asm("lock xchg [%1], %0": "=r"(value) : "r"(ptr) : "memory");
	return value;
}

/*
Ands a 16-bit value atomically
*/
static inline void atomicAnd16(uint16_t *ptr, uint16_t value) {
	asm("lock and [%1], %0": : "r"(value), "r"(ptr) : "memory");
}

/*
Ors a 16-bit value atomically
*/
static inline void atomicOr16(uint16_t *ptr, uint16_t value) {
	asm("lock or [%1], %0": : "r"(value), "r"(ptr) : "memory");
}

#endif