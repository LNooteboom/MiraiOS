#ifndef PAGING_H
#define PAGING_H

#include <global.h>


#define NROFPAGESINSTACK (PAGESIZE / PTRSIZE) - PTRSIZE


/*
A stack of free pages for allocating
*/
typedef struct pageStack {
	struct pageStack *next;
	void *pages[NROFPAGESINSTACK];
} pageStack_t;




#endif
