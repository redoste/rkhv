#include <stdbool.h>

#include <rkhv/rflags.h>
#include <rkhv/stdint.h>

#include "vm_emulated_instructions.h"
#include "vm_ioports.h"
#include "vmx_vmexit.h"

#define VM_EMULATED_INSTRUCTION_IO_SINGLE(direction, size)                                            \
	void vm_emulated_instruction_##direction##size(vmx_vmexit_state_t* vm_state, uint16_t port) { \
		vm_##direction##size(port, vm_state->reg_state->rax);                                 \
	}

VM_EMULATED_INSTRUCTION_IO_SINGLE(out, b)
VM_EMULATED_INSTRUCTION_IO_SINGLE(out, w)
VM_EMULATED_INSTRUCTION_IO_SINGLE(out, d)

#define VM_EMULATED_INSTRUCTION_IO_STR(direction, size, type)                                            \
	void vm_emulated_instruction_##direction##s##size(vmx_vmexit_state_t* vm_state, uint16_t port) { \
		bool df = vm_state->rflags & RFLAGS_DF;                                                  \
                                                                                                         \
		/* TODO : introduce bound checks when EPT is used */                                     \
		vm_##direction##size(port, *((type*)vm_state->reg_state->rsi));                          \
		vm_state->reg_state->rsi += df ? -sizeof(type) : sizeof(type);                           \
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
