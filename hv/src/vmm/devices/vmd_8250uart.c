#include <rkhv/mem.h>

#include <rkhv/arena.h>
#include <rkhv/panic.h>
#include <rkhv/serial.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vm_manager.h>

#include "vmd.h"
#include "vmd_8250uart.h"

// TODO : Implement a full 8250 and not just a dumb output
static void vmd_8250uart_outb_handler(vm_t* vm, void* device_data, uint16_t port, uint8_t read_byte) {
	if (port != COM1_DATA_REG) {
		PANIC("Reached 8250uart outb handler with unexpected port");
	}
	vmd_8250uart_t* uart = device_data;

	if (uart->output_buffer_size >= sizeof(uart->output_buffer) - 1) {
		// We should have already flushed it
		uart->output_buffer_size = 0;
	}
	uart->output_buffer[uart->output_buffer_size] = read_byte;
	uart->output_buffer_size++;

	if (read_byte == '\n' || uart->output_buffer_size >= sizeof(uart->output_buffer) - 1) {
		uart->output_buffer[uart->output_buffer_size] = 0;
		// TODO : trim or check for new line
		stdio_printf("\033[34;1mvm \"%s\"(COM1):\033[0m %s", vm->name, uart->output_buffer);
		uart->output_buffer_size = 0;
	}
}

vm_device_t* vmd_8250uart_create(void) {
	vm_device_t* device = arena_allocate(vmd_get_device_arena(), sizeof(vm_device_t));
	memset(device, 0, sizeof(*device));
	vmd_8250uart_t* uart = arena_allocate(vmd_get_device_arena(), sizeof(vmd_8250uart_t));
	memset(uart, 0, sizeof(*uart));

	device->ports[0] = COM1_DATA_REG;
	device->ports_len = 1;
	device->device_data = uart;
	device->outb_handler = vmd_8250uart_outb_handler;

	return device;
}
