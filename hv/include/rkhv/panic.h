#ifndef PANIC_H
#define PANIC_H

#define PANIC(message)                              \
	do {                                        \
		panic(message, __FILE__, __LINE__); \
	} while (0)

void __attribute__((noreturn)) panic(const char* message, const char* filename, unsigned int line_number);

#endif
