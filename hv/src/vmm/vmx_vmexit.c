#include <rkhv/stdint.h>

#define LOG_CATEGORY "vmx"
#include <rkhv/panic.h>
#include <rkhv/serial.h>
#include <rkhv/stdio.h>

#include "vmx_instructions.h"
#include "vmx_vmcs.h"
#include "vmx_vmexit.h"

static void vmx_vmexit_io(uint64_t exit_qualification, vmx_vmexit_reg_state_t* vm_reg_state) {
	static char vm_serial_buffer[128];
	static size_t vm_serial_buffer_size = 0;

	/* 0:2 size of access      : 0 = 1-byte
	 * 3   direction of access : 0 = out
	 * 4   string instruction
	 * 5   REP instruction
	 */
	if ((exit_qualification & 0x3f) != 0) {
		PANIC("unsupported I/O VM-exit qualification");
	}

	if (((exit_qualification >> 16) & 0xffff) != COM1_DATA_REG) {
		PANIC("unsupported I/O port");
	}

	uint8_t read_byte = vm_reg_state->rax & 0xff;
	if (vm_serial_buffer_size >= sizeof(vm_serial_buffer) - 1) {
		// We should have already flushed it
		vm_serial_buffer_size = 0;
	}
	vm_serial_buffer[vm_serial_buffer_size] = read_byte;
	vm_serial_buffer_size++;

	if (read_byte == '\n' || vm_serial_buffer_size >= sizeof(vm_serial_buffer) - 1) {
		vm_serial_buffer[vm_serial_buffer_size] = 0;
		// TODO : trim or check for new line
		stdio_printf("\033[34;1mvm(COM1):\033[0m %s", vm_serial_buffer);
		vm_serial_buffer_size = 0;
	}
}

void vmx_vmexit_handler(vmx_vmexit_reg_state_t* vm_reg_state) {
	uint64_t exit_reason, exit_qualification, guest_rip, instruction_length;
	VMX_ASSERT(vmx_vmread(VMCS_EXIT_REASON, &exit_reason));
	VMX_ASSERT(vmx_vmread(VMCS_EXIT_QUALIFICATION, &exit_qualification));
	VMX_ASSERT(vmx_vmread(VMCS_GUEST_RIP, &guest_rip));
	VMX_ASSERT(vmx_vmread(VMCS_VM_EXIT_INSTRUCTION_LENGTH, &instruction_length));

	switch (exit_reason) {
		case VMEXIT_REASON_IO_INSTRUCTION:
			vmx_vmexit_io(exit_qualification, vm_reg_state);
			VMX_ASSERT(vmx_vmwrite(VMCS_GUEST_RIP, guest_rip + instruction_length));
			return;

		case VMEXIT_REASON_HLT:
			LOG("VM-exit : VM halted");
			break;
		default:
			LOG("VM-exit : reason unsupported : %zxpq qualification=%zxpq @ rip=%zxpq", exit_reason, exit_qualification, guest_rip);
			break;
	}
	PANIC("vmexit_handler reached end");
}
