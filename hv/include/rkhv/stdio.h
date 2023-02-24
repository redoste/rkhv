#ifndef STDIO_H
#define STDIO_H

void stdio_puts(const char* str);
// See src/string.h for more details about the format string language
void stdio_printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#ifndef LOG_CATEGORY
#define LOG_CATEGORY "undefined"
#endif

#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(s, ...)                                                                        \
	do {                                                                               \
		stdio_printf("\033[33;1m" LOG_CATEGORY ":\033[0m " s "\n", ##__VA_ARGS__); \
	} while (0)

#endif
