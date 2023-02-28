#ifndef COMMON_RKHV_CR_MSR
#define COMMON_RKHV_CR_MSR

#include <stdint.h>

#define IA32_EFER           0xc0000080
#define IA32_VMX_BASIC      0x480
#define IA32_VMX_CR0_FIXED0 0x486
#define IA32_VMX_CR0_FIXED1 0x487
#define IA32_VMX_CR4_FIXED0 0x488
#define IA32_VMX_CR4_FIXED1 0x489

static inline uint64_t rdmsr(uint32_t msr) {
	uint32_t ret_high, ret_low;
	asm("rdmsr"
	    : "=d"(ret_high), "=a"(ret_low)
	    : "c"(msr));
	return ((uint64_t)ret_high << 32) | ret_low;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
	uint32_t value_high = value >> 32;
	uint32_t value_low = value & 0xffffffff;
	asm("wrmsr"
	    :
	    : "d"(value_high), "a"(value_low), "c"(msr));
}

/* NOTE : We need to `volatile` cr* register reads and writes because even with
 *        the clobber field set, clang wants to optimize away a second read
 */

static inline uint64_t cr0_read(void) {
	uint64_t ret;
	asm volatile("mov %%cr0, %0"
		     : "=r"(ret));
	return ret;
}

static inline void cr0_write(uint64_t value) {
	asm volatile("mov %0, %%cr0"
		     :
		     : "r"(value)
		     : "cr0");
}

static inline uint64_t cr4_read(void) {
	uint64_t ret;
	asm volatile("mov %%cr4, %0"
		     : "=r"(ret));
	return ret;
}

static inline void cr4_write(uint64_t value) {
	asm volatile("mov %0, %%cr4"
		     :
		     : "r"(value)
		     : "cr4");
}

#endif
