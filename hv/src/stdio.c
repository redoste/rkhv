#include <stdarg.h>

#include <rkhv/stdio.h>

#include "serial.h"
#include "string.h"

static inline void stdio_putc(const char c) {
	serial_write_byte(c);
}

void stdio_puts(const char* str) {
	while (*str) {
		stdio_putc(*(str++));
	}
}

void stdio_printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	str_printf_core(fmt, args, stdio_putc, stdio_puts);
	va_end(args);
}
