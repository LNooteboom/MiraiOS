#ifndef INCLUDE_MM_CACHE_H
#define INCLUDE_MM_CACHE_H

#include <stdint.h>
#include <sched/spinlock.h>

struct CachePage {
	struct CachePage *next;
	void *unused;
};

struct CacheEntry {
	struct CacheEntry *next;
};

struct Cache {
	const char *name;
	spinlock_t lock;
	uint32_t objSize;
	struct CacheEntry *list;
	struct CachePage *pList;
};

int cacheCreate(struct Cache *newCache, uint32_t size, const char *name);

void cachePurge(struct Cache *cache);

void *cacheGet(struct Cache *cache);

void cacheRelease(struct Cache *cache, void *obj);

#endif