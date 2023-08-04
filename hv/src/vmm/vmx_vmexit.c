#include <stdbool.h>
#include <stddef.h>

#include <rkhv/stdint.h>

#define LOG_CATEGORY "vmx"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>

#include "vm_emulated_instructions.h"
#include "vm_manager.h"
#include "vmx_instructions.h"
#include "vmx_msr.h"
#include "vmx_vmcs.h"
#include "vmx_vmexit.h"

static void vmx_vmexit_cr_accesses(uint64_t exit_qualification, vmx_vmexit_state_t* vm_state) {
	uint8_t crn = VMEXIT_QUALIFICATION_CR_CRN(exit_qualification);

	// TODO : factorize this lookup if it's used by another exit reason
	uint64_t* possible_gpr[] = {
		&vm_state->reg_state->rax,
		&vm_state->reg_state->rcx,
		&vm_state->reg_state->rdx,
		&vm_state->reg_state->rbx,
		&vm_state->rsp,
		&vm_state->reg_state->rbp,
		&vm_state->reg_state->rsi,
		&vm_state->reg_state->rdi,
		&vm_state->reg_state->r8,
		&vm_state->reg_state->r9,
		&vm_state->reg_state->r10,
		&vm_state->reg_state->r11,
		&vm_state->reg_state->r12,
		&vm_state->reg_state->r13,
		&vm_state->reg_state->r14,
		&vm_state->reg_state->r15,
	};
	uint64_t* gpr = possible_gpr[VMEXIT_QUALIFICATION_CR_MOV_GPR(exit_qualification)];

	if (VMEXIT_QUALIFICATION_CR_ACCESS_TYPE(exit_qualification) == VMEXIT_QUALIFICATION_CR_ACCESS_TYPE_MOV_TO_CR) {
		vm_emulated_instruction_mov_to_cr(vm_state, crn, *gpr);
	} else if (VMEXIT_QUALIFICATION_CR_ACCESS_TYPE(exit_qualification) == VMEXIT_QUALIFICATION_CR_ACCESS_TYPE_MOV_FROM_CR) {
		*gpr = vm_emulated_instruction_mov_from_cr(vm_state, crn);
	} else {
		LOG("VM-exit : qualification unsupported : qualification=%zxpq @ rip=%zxpq", exit_qualification, vm_state->rip);
		PANIC("unsupported CR VM-exit qualification");
	}
}

static void vmx_vmexit_io(uint64_t exit_qualification, vmx_vmexit_state_t* vm_state) {
	enum {
		EMULATED_INSTRUCTION_SINGLE,
		EMULATED_INSTRUCTION_STR,
		EMULATED_INSTRUCTION_STRREP,
	};

	typedef void (*emulated_io_instruction_t)(vmx_vmexit_state_t*, uint16_t);
	static const emulated_io_instruction_t emulated_io_instructions[2][4][3] = {
		[VMEXIT_QUALIFICATION_IO_DIRECTION_OUT] = {
			[VMEXIT_QUALIFICATION_IO_SIZE_BYTE] = {
				[EMULATED_INSTRUCTION_SINGLE] = vm_emulated_instruction_outb,
				[EMULATED_INSTRUCTION_STR] = vm_emulated_instruction_outsb,
				[EMULATED_INSTRUCTION_STRREP] = vm_emulated_instruction_rep_outsb,
			},
			[VMEXIT_QUALIFICATION_IO_SIZE_WORD] = {
				[EMULATED_INSTRUCTION_SINGLE] = vm_emulated_instruction_outw,
				[EMULATED_INSTRUCTION_STR] = vm_emulated_instruction_outsw,
				[EMULATED_INSTRUCTION_STRREP] = vm_emulated_instruction_rep_outsw,
			},
			[VMEXIT_QUALIFICATION_IO_SIZE_DWORD] = {
				[EMULATED_INSTRUCTION_SINGLE] = vm_emulated_instruction_outd,
				[EMULATED_INSTRUCTION_STR] = vm_emulated_instruction_outsd,
				[EMULATED_INSTRUCTION_STRREP] = vm_emulated_instruction_rep_outsd,
			},
		},
		[VMEXIT_QUALIFICATION_IO_DIRECTION_IN] = {
			[VMEXIT_QUALIFICATION_IO_SIZE_BYTE] = {
				[EMULATED_INSTRUCTION_SINGLE] = vm_emulated_instruction_inb,
				[EMULATED_INSTRUCTION_STR] = vm_emulated_instruction_insb,
				[EMULATED_INSTRUCTION_STRREP] = vm_emulated_instruction_rep_insb,
			},
			[VMEXIT_QUALIFICATION_IO_SIZE_WORD] = {
				[EMULATED_INSTRUCTION_SINGLE] = vm_emulated_instruction_inw,
				[EMULATED_INSTRUCTION_STR] = vm_emulated_instruction_insw,
				[EMULATED_INSTRUCTION_STRREP] = vm_emulated_instruction_rep_insw,
			},
			[VMEXIT_QUALIFICATION_IO_SIZE_DWORD] = {
				[EMULATED_INSTRUCTION_SINGLE] = vm_emulated_instruction_ind,
				[EMULATED_INSTRUCTION_STR] = vm_emulated_instruction_insd,
				[EMULATED_INSTRUCTION_STRREP] = vm_emulated_instruction_rep_insd,
			},
		},
	};

	bool string = VMEXIT_QUALIFICATION_IO_STRING(exit_qualification);
	bool rep = VMEXIT_QUALIFICATION_IO_REP(exit_qualification);
	const emulated_io_instruction_t* emulated_io_instructions_base = emulated_io_instructions
		[VMEXIT_QUALIFICATION_IO_DIRECTION(exit_qualification)]
		[VMEXIT_QUALIFICATION_IO_SIZE(exit_qualification)];
	emulated_io_instruction_t emulated_io_instruction = NULL;
	if (!string && !rep) {
		emulated_io_instruction = emulated_io_instructions_base[EMULATED_INSTRUCTION_SINGLE];
	} else if (string && !rep) {
		emulated_io_instruction = emulated_io_instructions_base[EMULATED_INSTRUCTION_STR];
	} else if (string && rep) {
		emulated_io_instruction = emulated_io_instructions_base[EMULATED_INSTRUCTION_STRREP];
	}

	if (emulated_io_instruction == NULL) {
		LOG("VM-exit : qualification unsupported : qualification=%zxpq @ rip=%zxpq", exit_qualification, vm_state->rip);
		PANIC("unsupported I/O VM-exit qualification");
	}

	emulated_io_instruction(vm_state, VMEXIT_QUALIFICATION_IO_PORT(exit_qualification));
}

vm_permanent_state_t* vmx_vmexit_handler(vmx_vmexit_reg_state_t* vm_reg_state, void* vm_xsave_area) {
	uint64_t exit_reason, exit_qualification, instruction_length;
	VMX_ASSERT(vmx_vmread(VMCS_EXIT_REASON, &exit_reason));
	VMX_ASSERT(vmx_vmread(VMCS_EXIT_QUALIFICATION, &exit_qualification));
	VMX_ASSERT(vmx_vmread(VMCS_VM_EXIT_INSTRUCTION_LENGTH, &instruction_length));

	vmx_vmexit_state_t vm_state = {
		.vm = vm_manager_get_current_vm(),
		.reg_state = vm_reg_state,
		.xsave_area = vm_xsave_area,
	};
	VMX_ASSERT(vmx_vmread(VMCS_GUEST_RIP, &vm_state.rip));
	VMX_ASSERT(vmx_vmread(VMCS_GUEST_RSP, &vm_state.rsp));
	VMX_ASSERT(vmx_vmread(VMCS_GUEST_RFLAGS, &vm_state.rflags));
	VMX_ASSERT(vmx_vmread(VMCS_GUEST_CR0, &vm_state.cr0));
	VMX_ASSERT(vmx_vmread(VMCS_GUEST_CR3, &vm_state.cr3));
	VMX_ASSERT(vmx_vmread(VMCS_GUEST_CR4, &vm_state.cr4));
	vmx_msr_vmexit(&vm_state);

	switch (exit_reason) {
		case VMEXIT_REASON_CPUID:
			vm_emulated_instruction_cpuid(&vm_state);
			vm_state.rip += instruction_length;
			break;

		case VMEXIT_REASON_CR_ACCESSES:
			vmx_vmexit_cr_accesses(exit_qualification, &vm_state);
			vm_state.rip += instruction_length;
			break;

		case VMEXIT_REASON_IO_INSTRUCTION:
			vmx_vmexit_io(exit_qualification, &vm_state);
			vm_state.rip += instruction_length;
			break;

		case VMEXIT_REASON_RDMSR:
			LOG("VM-exit : RDMSR with rcx=%zxpq", vm_state.reg_state->rcx);
			PANIC("unsupported MSR");
			break;

		case VMEXIT_REASON_WRMSR:
			LOG("VM-exit : WRMSR with rcx=%zxpq", vm_state.reg_state->rcx);
			PANIC("unsupported MSR");
			break;

		case VMEXIT_REASON_HLT:
			LOG("VM-exit : VM \"%s\" halted @ rip=%zxpq", vm_state.vm->name, vm_state.rip);
			PANIC("VM halted");
			break;

		default:
			LOG("VM-exit : reason unsupported : %zxpq qualification=%zxpq @ rip=%zxpq",
			    exit_reason, exit_qualification, vm_state.rip);
			PANIC("Unsupported VM-Exit reason");
			break;
	}
	VMX_ASSERT(vmx_vmwrite(VMCS_GUEST_RIP, vm_state.rip));
	VMX_ASSERT(vmx_vmwrite(VMCS_GUEST_RSP, vm_state.rsp));
	VMX_ASSERT(vmx_vmwrite(VMCS_GUEST_RFLAGS, vm_state.rflags));
	VMX_ASSERT(vmx_vmwrite(VMCS_GUEST_CR0, vm_state.cr0));
	VMX_ASSERT(vmx_vmwrite(VMCS_GUEST_CR3, vm_state.cr3));
	VMX_ASSERT(vmx_vmwrite(VMCS_GUEST_CR4, vm_state.cr4));
	vmx_msr_vmresume(&vm_state);

	return &vm_state.vm->permanent_state;
}
