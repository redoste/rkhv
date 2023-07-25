#ifndef COMMON_XSAVE_H
#define COMMON_XSAVE_H

#include <rkhv/stdint.h>

extern uint64_t xsave_host_xcr0;
extern size_t xsave_area_size;

static inline void xsetbv(uint64_t xcr0) {
	uint32_t ecx = 0;
	uint32_t xcr0_high = xcr0 >> 32;
	uint32_t xcr0_low = xcr0 & 0xffffffff;
	asm volatile("xsetbv"
		     :
		     : "c"(ecx), "d"(xcr0_high), "a"(xcr0_low));
}

static inline uint64_t xgetbv(void) {
	uint32_t ecx = 0;
	uint32_t xcr0_high, xcr0_low;
	asm volatile("xgetbv"
		     : "=d"(xcr0_high), "=a"(xcr0_low)
		     : "c"(ecx));
	return ((uint64_t)xcr0_high << 32) | xcr0_low;
}

#endif
