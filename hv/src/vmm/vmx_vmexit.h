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

typedef struct vmx_vmexit_state_t {
	vmx_vmexit_reg_state_t* reg_state;
	uintptr_t rip;
	uintptr_t rflags;
} vmx_vmexit_state_t;

void vmx_vmexit(void);
void vmx_vmexit_handler(vmx_vmexit_reg_state_t* vm_reg_state);

#define VMEXIT_REASON_HLT            12
#define VMEXIT_REASON_IO_INSTRUCTION 30

#define VMEXIT_QUALIFICATION_IO_SIZE(x)       ((x)&3)
#define VMEXIT_QUALIFICATION_IO_SIZE_BYTE     0
#define VMEXIT_QUALIFICATION_IO_SIZE_WORD     1
#define VMEXIT_QUALIFICATION_IO_SIZE_DWORD    3
#define VMEXIT_QUALIFICATION_IO_DIRECTION(x)  (((x) >> 3) & 1)
#define VMEXIT_QUALIFICATION_IO_DIRECTION_OUT 0
#define VMEXIT_QUALIFICATION_IO_DIRECTION_IN  1
#define VMEXIT_QUALIFICATION_IO_STRING(x)     (((x) >> 4) & 1)
#define VMEXIT_QUALIFICATION_IO_REP(x)        (((x) >> 5) & 1)
#define VMEXIT_QUALIFICATION_IO_OPERAND(x)    (((x) >> 6) & 1)
#define VMEXIT_QUALIFICATION_IO_OPERAND_DX    0
#define VMEXIT_QUALIFICATION_IO_OPERAND_IMM   1
#define VMEXIT_QUALIFICATION_IO_PORT(x)       (((x) >> 16) & 0xffff)

#endif
