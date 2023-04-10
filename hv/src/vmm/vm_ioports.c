#include <rkhv/stdint.h>

#define LOG_CATEGORY "vm"
#include <rkhv/panic.h>
#include <rkhv/serial.h>
#include <rkhv/stdio.h>

#include "vm_ioports.h"

static void vm_com1_write(uint8_t read_byte) {
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
		stdio_printf("\033[34;1mvm(COM1):\033[0m %s", vm_serial_buffer);
		vm_serial_buffer_size = 0;
	}
}

void vm_outb(uint16_t port, uint8_t value) {
	if (port != COM1_DATA_REG) {
		LOG("outb port=%xpw value=%xpb", port, value);
		PANIC("unsupported I/O port");
	}

	vm_com1_write(value);
}

void vm_outw(uint16_t port, uint16_t value) {
	LOG("outw port=%xpw value=%xpw", port, value);
	PANIC("unsupported I/O port");
}

void vm_outd(uint16_t port, uint32_t value) {
	LOG("outd port=%xpw value=%xpd", port, value);
	PANIC("unsupported I/O port");
}
