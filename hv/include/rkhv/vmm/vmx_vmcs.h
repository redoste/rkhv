#ifndef COMMON_VMX_VMCS
#define COMMON_VMX_VMCS

#include <rkhv/stdint.h>

#include <rkhv/interrupts.h>
#include <rkhv/segments.h>

typedef struct vmx_initial_vmcs_config_t {
	uintptr_t eptp;
	struct {
		uintptr_t rip;
		uintptr_t rsp;
		uintptr_t cr0;
		uintptr_t cr3;
		uintptr_t cr4;
		uint64_t ia32_efer;
		uint16_t cs;
		uint16_t ds;
		gdtr_t gdtr;
		idtr_t idtr;
	} guest_state;
} vmx_initial_vmcs_config_t;

/* NOTE : For now this structure is compatible with vmx_vmexit_state_t but it
 *        might change in the future
 */
typedef struct vmx_initial_gpr_state_t {
	uintptr_t r15;
	uintptr_t r14;
	uintptr_t r13;
	uintptr_t r12;
	uintptr_t r11;
	uintptr_t r10;
	uintptr_t r9;
	uintptr_t r8;
	uintptr_t rdi;
	uintptr_t rsi;
	uintptr_t rbp;
	uintptr_t rdx;
	uintptr_t rcx;
	uintptr_t rbx;
	uintptr_t rax;
} vmx_initial_gpr_state_t;

#endif
