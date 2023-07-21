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

#endif
