#ifndef INCLUDEPAGING_H
#define INCLUDEPAGING_H

#define PAGESIZE 4096

/*
Allocates a 4kb page and returns its virtual address
*/
void *allocPhysPage(void);

/*
Deallocates a 4kb page
*/
void deallocPhysPage(void *page);

#endif
