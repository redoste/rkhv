#include <stdbool.h>

#include <rkhv/cr_msr.h>
#include <rkhv/rflags.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "vm"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>
#include <rkhv/xsave.h>

#include "vm_emulated_instructions.h"
#include "vm_guest_paging.h"
#include "vm_ioports.h"
#include "vmx_instructions.h"
#include "vmx_vmcs.h"
#include "vmx_vmexit.h"

void vm_emulated_instruction_cpuid(vmx_vmexit_state_t* vm_state) {
	if (vm_state->reg_state->rax >= 0x40000000 && vm_state->reg_state->rax <= 0x4FFFFFFF) {
		/* These invalid values are used by other hypervisors to enable paravirtualization
		 * We intentionally suppress them since we are probably running under another hypervisor during development
		 */
		return;
	}

	// TODO : make CPUID.(EAX=0DH,ECX=0).EBX dependent of guest XCR0
	// TODO : add a way to customize `cpuid`
	asm("cpuid"
	    : "=a"(vm_state->reg_state->rax), "=b"(vm_state->reg_state->rbx), "=c"(vm_state->reg_state->rcx), "=d"(vm_state->reg_state->rdx)
	    : "a"(vm_state->reg_state->rax), "b"(vm_state->reg_state->rbx), "c"(vm_state->reg_state->rcx), "d"(vm_state->reg_state->rdx));
}

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

#define VM_EMULATED_INSTRUCTION_OUT_SINGLE(size)                                              \
	void vm_emulated_instruction_out##size(vmx_vmexit_state_t* vm_state, uint16_t port) { \
		vm_out##size(vm_state->vm, port, vm_state->reg_state->rax);                   \
	}

VM_EMULATED_INSTRUCTION_OUT_SINGLE(b)
VM_EMULATED_INSTRUCTION_OUT_SINGLE(w)
VM_EMULATED_INSTRUCTION_OUT_SINGLE(d)

#define VM_EMULATED_INSTRUCTION_IN_SINGLE(size, mask)                                        \
	void vm_emulated_instruction_in##size(vmx_vmexit_state_t* vm_state, uint16_t port) { \
		vm_state->reg_state->rax = (vm_state->reg_state->rax & (mask)) |             \
					   vm_in##size(vm_state->vm, port);                  \
	}

VM_EMULATED_INSTRUCTION_IN_SINGLE(b, ~0xff)
VM_EMULATED_INSTRUCTION_IN_SINGLE(w, ~0xffff)
VM_EMULATED_INSTRUCTION_IN_SINGLE(d, ~0xffffffff)

#define VM_EMULATED_INSTRUCTION_OUT_STR(size, type)                                                                                      \
	void vm_emulated_instruction_outs##size(vmx_vmexit_state_t* vm_state, uint16_t port) {                                           \
		bool df = vm_state->rflags & RFLAGS_DF;                                                                                  \
                                                                                                                                         \
		type* hostva = vm_guest_paging_get_host_virtual_address_during_vmexit(vm_state, vm_state->reg_state->rsi, sizeof(type)); \
		vm_out##size(vm_state->vm, port, *hostva);                                                                               \
		vm_state->reg_state->rsi += df ? -sizeof(type) : sizeof(type);                                                           \
	}

VM_EMULATED_INSTRUCTION_OUT_STR(b, uint8_t)
VM_EMULATED_INSTRUCTION_OUT_STR(w, uint16_t)
VM_EMULATED_INSTRUCTION_OUT_STR(d, uint32_t)

#define VM_EMULATED_INSTRUCTION_IN_STR(size, type)                                                                                       \
	void vm_emulated_instruction_ins##size(vmx_vmexit_state_t* vm_state, uint16_t port) {                                            \
		bool df = vm_state->rflags & RFLAGS_DF;                                                                                  \
                                                                                                                                         \
		type* hostva = vm_guest_paging_get_host_virtual_address_during_vmexit(vm_state, vm_state->reg_state->rdi, sizeof(type)); \
		*hostva = vm_in##size(vm_state->vm, port);                                                                               \
		vm_state->reg_state->rdi += df ? -sizeof(type) : sizeof(type);                                                           \
	}

VM_EMULATED_INSTRUCTION_IN_STR(b, uint8_t)
VM_EMULATED_INSTRUCTION_IN_STR(w, uint16_t)
VM_EMULATED_INSTRUCTION_IN_STR(d, uint32_t)

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
VM_EMULATED_INSTRUCTION_IO_STRREP(in, b)
VM_EMULATED_INSTRUCTION_IO_STRREP(in, w)
VM_EMULATED_INSTRUCTION_IO_STRREP(in, d)

void vm_emulated_instruction_xsetbv(vmx_vmexit_state_t* vm_state) {
	if (vm_state->reg_state->rcx != 0) {
		PANIC("unsupported XCR specified by RCX on emulated XSETBV");
	}

	uint64_t new_xcr0 = (vm_state->reg_state->rdx << 32) | (vm_state->reg_state->rax & 0xffffffff);
	if ((new_xcr0 & 1) != 1) {
		PANIC("new guest XCR0 doesn't have bit 0 set");
	}
	if ((new_xcr0 & (~xsave_host_xcr0)) != 0 ||
	    (new_xcr0 & 0x6) == 0x4)  // XCR0[2:1] == 0b10
	{
		PANIC("new guest XCR0 set unsupported bits");
	}

	// TODO : We should replace these PANICs for a #GP
	// TODO : We should probably check the CPL

	vm_state->vm->permanent_state.xcr0 = new_xcr0;
}
