#include <stdint.h>

void *testString = ("H\0e\0l\0l\0o\0 \0f\0r\0o\0m\0 \0C\0!\0");

void efiMain(void *imageHandler, uint64_t *efiSystemTable) {
	asm volatile (
		"sub rsp, 40;"
		"mov rcx, [rdx+64];"
		"mov rdx, rdi;"
		"call [rcx+8];"
		"add rsp, 40;" : : "d"(efiSystemTable), "D"(testString));
	while (1);
}