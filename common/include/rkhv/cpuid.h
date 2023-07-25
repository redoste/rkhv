#ifndef COMMON_CPUID_H
#define COMMON_CPUID_H

#include <stddef.h>

#include <rkhv/stdint.h>

static inline void cpuid(uint32_t input_eax, uint32_t input_ecx, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
	uint32_t ignored;
	if (eax == NULL) {
		eax = &ignored;
	}
	if (ebx == NULL) {
		ebx = &ignored;
	}
	if (ecx == NULL) {
		ecx = &ignored;
	}
	if (edx == NULL) {
		edx = &ignored;
	}

	asm volatile("cpuid"
		     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
		     : "a"(input_eax), "c"(input_ecx));
}

#endif
