#ifndef INCLUDE_MODULES_H
#define INCLUDE_MODULES_H

#define NROF_MODULE_LEVELS		4

#define MODULE_LEVEL_EARLY		0
#define MODULE_LEVEL_SCHED		1
#define MODULE_LEVEL_FS		2
#define MODULE_LEVEL_DEVICE	3

typedef int(*moduleCall_t)(void);

/*
Adds a function to an initializer list
functions with a lower level will be initialized first
*/
#define MODULE_INIT_LEVEL(func, lv)	\
	static moduleCall_t __moduleInit_##func __attribute__((used, section (".moduleInits" #lv))) = func

#define MODULE_INIT(func)	MODULE_INIT_LEVEL(func, 3)

#endif