#ifndef COMMON_RKHV_CR_MSR
#define COMMON_RKHV_CR_MSR

#include <stdint.h>

#define IA32_BIOS_SIGN_ID   0x8B
#define IA32_MISC_ENABLE    0x1A0
#define IA32_XSS            0xDA0
#define IA32_EFER           0xC0000080
#define IA32_FS_BASE        0xC0000100
#define IA32_GS_BASE        0xC0000101
#define IA32_KERNEL_GS_BASE 0xC0000102

#define IA32_VMX_BASIC           0x480
#define IA32_VMX_PINBASED_CTLS   0x481
#define IA32_VMX_PROCBASED_CTLS  0x482
#define IA32_VMX_EXIT_CTLS       0x483
#define IA32_VMX_CR0_FIXED0      0x486
#define IA32_VMX_ENTRY_CTLS      0x484
#define IA32_VMX_CR0_FIXED1      0x487
#define IA32_VMX_CR4_FIXED0      0x488
#define IA32_VMX_CR4_FIXED1      0x489
#define IA32_VMX_EPT_VPID_CAP    0x48C
#define IA32_VMX_PROCBASED_CTLS2 0x48B

#define CR0_PE (1 << 0)
#define CR0_MP (1 << 1)
#define CR0_EM (1 << 2)
#define CR0_TS (1 << 3)
#define CR0_ET (1 << 4)
#define CR0_NE (1 << 5)
#define CR0_WP (1 << 16)
#define CR0_AM (1 << 18)
#define CR0_NW (1 << 29)
#define CR0_CD (1 << 30)
#define CR0_PG (1 << 31)

#define CR4_VME        (1 << 0)
#define CR4_PVI        (1 << 1)
#define CR4_TSD        (1 << 2)
#define CR4_DE         (1 << 3)
#define CR4_PSE        (1 << 4)
#define CR4_PAE        (1 << 5)
#define CR4_MCE        (1 << 6)
#define CR4_PGE        (1 << 7)
#define CR4_PCE        (1 << 8)
#define CR4_OSFXSR     (1 << 9)
#define CR4_OSXMMEXCPT (1 << 10)
#define CR4_UMIP       (1 << 11)
#define CR4_LA57       (1 << 12)
#define CR4_VMXE       (1 << 13)
#define CR4_SMXE       (1 << 14)
#define CR4_FSGSBASE   (1 << 16)
#define CR4_PCIDE      (1 << 17)
#define CR4_OSXSAVE    (1 << 18)
#define CR4_KL         (1 << 19)
#define CR4_SMEP       (1 << 20)
#define CR4_SMAP       (1 << 21)
#define CR4_PKE        (1 << 22)
#define CR4_CET        (1 << 23)
#define CR4_PKS        (1 << 24)
#define CR4_UINTR      (1 << 25)

#define IA32_EFER_SCE (1 << 0)
#define IA32_EFER_LME (1 << 8)
#define IA32_EFER_LMA (1 << 10)
#define IA32_EFER_NXE (1 << 11)

/* NOTE : We need to `volatile` these `asm` statements because even with
 *        the clobber field set, clang wants to optimize away a second read
 */

static inline uint64_t rdmsr(uint32_t msr) {
	uint32_t ret_high, ret_low;
	asm volatile("rdmsr"
		     : "=d"(ret_high), "=a"(ret_low)
		     : "c"(msr));
	return ((uint64_t)ret_high << 32) | ret_low;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
	uint32_t value_high = value >> 32;
	uint32_t value_low = value & 0xffffffff;
	asm volatile("wrmsr"
		     :
		     : "d"(value_high), "a"(value_low), "c"(msr));
}

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

static inline uint64_t cr3_read(void) {
	uint64_t ret;
	asm volatile("mov %%cr3, %0"
		     : "=r"(ret));
	return ret;
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
