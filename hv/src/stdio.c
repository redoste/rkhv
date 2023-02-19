#include "stdio.h"
#include "serial.h"

void stdio_puts(const char* str) {
	while (*str) {
		serial_putc(*(str++));
	}
}
