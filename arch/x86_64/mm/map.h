#ifndef MAP_H
#define MAP_H

#include <global.h>
#include <mm/paging.h>

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
