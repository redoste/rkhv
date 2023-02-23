#ifndef COMMON_RKHV_IO_PORTS_H
#define COMMON_RKHV_IO_PORTS_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm("inb %1, %0"
	    : "=a"(ret)
	    : "d"(port));
	return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
	asm("outb %0, %1"
	    :
	    : "a"(value), "d"(port));
}

#endif
