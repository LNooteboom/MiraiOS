#include <mm/cache.h>
#include <mm/memset.h>
#include <mm/paging.h>
#include <errno.h>
#include <print.h>

#define CACHE_NPAGES	1

int cacheCreate(struct Cache *newCache, uint32_t size, const char *name) {
	size = (size < 16)? 16 : align(size, 16);
	if (size > 1024) {
		printk("Too big for cache\n");
		return -EINVAL;
	}
	memset(newCache, 0, sizeof(*newCache));
	newCache->name = name;
	newCache->objSize = size;
	return 0;
}

void cachePurge(struct Cache *cache) {
	acquireSpinlock(&cache->lock);
	struct CachePage *page = cache->pList;
	while (page) {
		struct CachePage *next = page->next;
		deallocPages(page, CACHE_NPAGES*PAGE_SIZE);
		page = next;
	}
	cache->pList = NULL;
	cache->list = NULL;
	releaseSpinlock(&cache->lock);
}

static int addNewPage(struct Cache *cache) {
	struct CachePage *page = allocKPages(CACHE_NPAGES*PAGE_SIZE, PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
	if (!page) {
		return -ENOMEM;
	}
	page->next = cache->pList;
	cache->pList = page;

	uintptr_t p2 = (uintptr_t)(page + 1);
	uint32_t nrofEntries = (CACHE_NPAGES*PAGE_SIZE - sizeof(struct CachePage)) / cache->objSize;
	for (uint32_t i = 0; i < nrofEntries; i++) {
		struct CacheEntry *en = (struct CacheEntry *)(p2);
		en->next = cache->list;
		cache->list = en;
		p2 += cache->objSize;
	}

	return 0;
}

void *cacheGet(struct Cache *cache) {
	//printk("get\n");
	acquireSpinlock(&cache->lock);
	struct CacheEntry *en = cache->list;
	if (!en) {
		if (addNewPage(cache)) {
			return NULL;
		}
		en = cache->list;
	}
	cache->list = en->next;
	releaseSpinlock(&cache->lock);
	return en;
}

void cacheRelease(struct Cache *cache, void *obj) {
	acquireSpinlock(&cache->lock);
	struct CacheEntry *en = obj;
	en->next = cache->list;
	cache->list = en;
	releaseSpinlock(&cache->lock);
}