#ifndef INCLUDE_GLOBAL_H
#define INCLUDE_GLOBAL_H

#define NULL 0


//signed integer types
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long int64_t;
//unsigned integer types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef uint64_t uintptr_t;

typedef uint64_t size_t;

typedef enum {false, true} bool;

extern char VMEM_OFFSET;

#endif
