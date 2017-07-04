#ifndef INCLUDE_MODULES_H
#define INCLUDE_MODULES_H

typedef int(*moduleCall_t)(void);

#define MODULE_INIT_LEVEL(func, lv)	\
	static moduleCall_t __moduleInit_##func __attribute__((used, section (".moduleInits" #lv ))) = func

#endif