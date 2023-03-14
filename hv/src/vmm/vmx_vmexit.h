#ifndef VMX_VMEXIT
#define VMX_VMEXIT

#include <rkhv/stdint.h>

typedef struct vmx_vmexit_reg_state_t {
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
} vmx_vmexit_reg_state_t;

void vmx_vmexit(void);
void vmx_vmexit_handler(vmx_vmexit_reg_state_t* vm_reg_state);

#define VMEXIT_REASON_HLT            12
#define VMEXIT_REASON_IO_INSTRUCTION 30

#endif
