#ifndef INCLUDE_ATOMIC_H
#define INCLUDE_ATOMIC_H

static inline uint16_t atomicXchg16(uint16_t *ptr, uint16_t value) {
	asm("lock xchg [%1], %0": "=r"(value) : "r"(ptr));
	return value;
}

static inline void atomicAnd16(uint16_t *ptr, uint16_t value) {
	asm("lock and [%1], %0": : "r"(value), "r"(ptr));
}

static inline void atomicOr16(uint16_t *ptr, uint16_t value) {
	asm("lock or [%1], %0": : "r"(value), "r"(ptr));
}

#endif