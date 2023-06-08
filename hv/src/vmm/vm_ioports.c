#include <rkhv/stdint.h>

#define LOG_CATEGORY "vm"
#include <rkhv/panic.h>
#include <rkhv/serial.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vm_manager.h>

#include "vm_ioports.h"

static void vm_com1_write(vm_t* vm, uint8_t read_byte) {
	static char vm_serial_buffer[128];
	static size_t vm_serial_buffer_size = 0;

	if (vm_serial_buffer_size >= sizeof(vm_serial_buffer) - 1) {
		// We should have already flushed it
		vm_serial_buffer_size = 0;
	}
	vm_serial_buffer[vm_serial_buffer_size] = read_byte;
	vm_serial_buffer_size++;

	if (read_byte == '\n' || vm_serial_buffer_size >= sizeof(vm_serial_buffer) - 1) {
		vm_serial_buffer[vm_serial_buffer_size] = 0;
		// TODO : trim or check for new line
		stdio_printf("\033[34;1mvm \"%s\"(COM1):\033[0m %s", vm->name, vm_serial_buffer);
		vm_serial_buffer_size = 0;
	}
}

void vm_outb(vm_t* vm, uint16_t port, uint8_t value) {
	if (port != COM1_DATA_REG) {
		LOG("outb \"%s\" port=%xpw value=%xpb", vm->name, port, value);
		PANIC("unsupported I/O port");
	}

	vm_com1_write(vm, value);
}

void vm_outw(vm_t* vm, uint16_t port, uint16_t value) {
	LOG("outw \"%s\" port=%xpw value=%xpw", vm->name, port, value);
	PANIC("unsupported I/O port");
}

void vm_outd(vm_t* vm, uint16_t port, uint32_t value) {
	LOG("outd \"%s\" port=%xpw value=%xpd", vm->name, port, value);
	PANIC("unsupported I/O port");
}
