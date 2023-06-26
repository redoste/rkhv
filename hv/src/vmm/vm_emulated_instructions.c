#include <stdbool.h>

#include <rkhv/cr_msr.h>
#include <rkhv/rflags.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "vm"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>

#include "vm_emulated_instructions.h"
#include "vm_guest_paging.h"
#include "vm_ioports.h"
#include "vmx_instructions.h"
#include "vmx_vmcs.h"
#include "vmx_vmexit.h"

void vm_emulated_instruction_mov_to_cr(vmx_vmexit_state_t* vm_state, uint8_t crn, uint64_t new_cr) {
	if (crn == 3) {
		vm_state->cr3 = new_cr;
	} else if (crn == 4) {
		/* CR4.VMXE must remain set as it is required by IA32_VMX_CR4_FIXED but the CR4 read shadow
		 * allows us to "fake" a read where the bit is clear
		 * See Intel Manual Volume 3 : Paragraph 25.6.6 : Guest/Host Masks and Read Shadows for CR0 and CR4
		 */
		vm_state->cr4 = new_cr | CR4_VMXE;
		VMX_ASSERT(vmx_vmwrite(VMCS_CF_CR4_READ_SHADOW, new_cr & CR4_VMXE));
	} else {
		LOG("MOV to CR%u rip=%zxpq", crn, vm_state->rip);
		PANIC("unsupported CR in emulated MOV to CR");
	}
}

uint64_t vm_emulated_instruction_mov_from_cr(vmx_vmexit_state_t* vm_state, uint8_t crn) {
	if (crn == 3) {
		return vm_state->cr3;
	} else {
		LOG("MOV from CR%u rip=%zxpq", crn, vm_state->rip);
		PANIC("unsupported CR in emulated MOV from CR");
	}
}

#define VM_EMULATED_INSTRUCTION_IO_SINGLE(direction, size)                                            \
	void vm_emulated_instruction_##direction##size(vmx_vmexit_state_t* vm_state, uint16_t port) { \
		vm_##direction##size(vm_state->vm, port, vm_state->reg_state->rax);                   \
	}

VM_EMULATED_INSTRUCTION_IO_SINGLE(out, b)
VM_EMULATED_INSTRUCTION_IO_SINGLE(out, w)
VM_EMULATED_INSTRUCTION_IO_SINGLE(out, d)

#define VM_EMULATED_INSTRUCTION_IO_STR(direction, size, type)                                                                            \
	void vm_emulated_instruction_##direction##s##size(vmx_vmexit_state_t* vm_state, uint16_t port) {                                 \
		bool df = vm_state->rflags & RFLAGS_DF;                                                                                  \
                                                                                                                                         \
		type* hostva = vm_guest_paging_get_host_virtual_address_during_vmexit(vm_state, vm_state->reg_state->rsi, sizeof(type)); \
		vm_##direction##size(vm_state->vm, port, *hostva);                                                                       \
		vm_state->reg_state->rsi += df ? -sizeof(type) : sizeof(type);                                                           \
	}

VM_EMULATED_INSTRUCTION_IO_STR(out, b, uint8_t)
VM_EMULATED_INSTRUCTION_IO_STR(out, w, uint16_t)
VM_EMULATED_INSTRUCTION_IO_STR(out, d, uint32_t)

#define VM_EMULATED_INSTRUCTION_IO_STRREP(direction, size)                                                   \
	void vm_emulated_instruction_rep_##direction##s##size(vmx_vmexit_state_t* vm_state, uint16_t port) { \
		while (vm_state->reg_state->rcx != 0) {                                                      \
			vm_emulated_instruction_##direction##s##size(vm_state, port);                        \
			vm_state->reg_state->rcx--;                                                          \
		}                                                                                            \
	}

VM_EMULATED_INSTRUCTION_IO_STRREP(out, b)
VM_EMULATED_INSTRUCTION_IO_STRREP(out, w)
VM_EMULATED_INSTRUCTION_IO_STRREP(out, d)
