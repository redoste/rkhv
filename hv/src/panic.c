#include <rkhv/stdint.h>

#define LOG_CATEGORY "panic"
#include "stdio.h"

#include "panic.h"

void __attribute__((noreturn)) panic(const char* message, const char* filename, unsigned int line_number) {
	asm("cli");

	stdio_puts("\033[31;1mPANIC PANIC PANIC\033[0m\n");
	LOG("%s:%u: %s", filename, line_number, message);

	LOG("calling int 3");
	asm("int3");
	while (1) {
		asm("hlt");
	}
}
