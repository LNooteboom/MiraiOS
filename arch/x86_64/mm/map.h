#ifndef MAP_H
#define MAP_H

#include <global.h>
#include <mm/paging.h>

#define MMU_FLAG_PRESENT 	(1 << 0)
#define MMU_FLAG_WRITE 		(1 << 1)
#define MMU_FLAG_USER 		(1 << 2)
#define MMU_FLAG_SIZE		(1 << 7)

typedef uint64_t pte_t;

/*
Finds the entry in the page table at a specified level and sets it to a specified value.
*/
void mmSetPageEntry(uintptr_t addr, uint8_t level, pte_t entry);

/*
Finds the entry in the page table at a specified level and sets it to a specfied value if there is no existing entry there.
*/
void mmSetPageEntryIfNotExists(uintptr_t addr, uint8_t level, pte_t entry);

/*
Finds the entry in the page table at a specified level and returns it.
*/
pte_t *mmGetEntry(uintptr_t addr, uint8_t level);

#endif
