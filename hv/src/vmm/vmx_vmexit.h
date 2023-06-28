#ifndef VMX_VMEXIT
#define VMX_VMEXIT

#include <rkhv/stdint.h>

#include <rkhv/vmm/vm_manager.h>

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
	vm_t* vm;
	vmx_vmexit_reg_state_t* reg_state;
	uintptr_t rip;
	uintptr_t rsp;
	uintptr_t rflags;

	uint64_t cr0;
	uint64_t cr3;
	uint64_t cr4;
	uint64_t ia32_efer;
} vmx_vmexit_state_t;

void vmx_vmexit(void);
void vmx_vmexit_handler(vmx_vmexit_reg_state_t* vm_reg_state);

#define VMEXIT_REASON_CPUID          10
#define VMEXIT_REASON_HLT            12
#define VMEXIT_REASON_CR_ACCESSES    28
#define VMEXIT_REASON_IO_INSTRUCTION 30

#define VMEXIT_QUALIFICATION_CR_CRN(x)                  ((x)&0xf)
#define VMEXIT_QUALIFICATION_CR_ACCESS_TYPE(x)          (((x) >> 4) & 3)
#define VMEXIT_QUALIFICATION_CR_ACCESS_TYPE_MOV_TO_CR   0
#define VMEXIT_QUALIFICATION_CR_ACCESS_TYPE_MOV_FROM_CR 1
#define VMEXIT_QUALIFICATION_CR_ACCESS_TYPE_CLTS        2
#define VMEXIT_QUALIFICATION_CR_ACCESS_TYPE_LMSW        3
#define VMEXIT_QUALIFICATION_CR_LMSW_OPERAND(x)         (((x) >> 6) & 1)
#define VMEXIT_QUALIFICATION_CR_LMSW_OPERAND_REG        0
#define VMEXIT_QUALIFICATION_CR_LMSW_OPERAND_MEM        1
#define VMEXIT_QUALIFICATION_CR_MOV_GPR(x)              (((x) >> 8) & 0xf)
#define VMEXIT_QUALIFICATION_CR_LMSW_SRC(x)             (((x) >> 16) & 0xffff)

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
