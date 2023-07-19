#include <stdbool.h>

#include <rkhv/assert.h>
#include <rkhv/mem.h>

#define LOG_CATEGORY "vmd_8250uart"
#include <rkhv/arena.h>
#include <rkhv/panic.h>
#include <rkhv/serial.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vm_manager.h>

#include "vmd.h"
#include "vmd_8250uart.h"

/* References : https://www.sci.muni.cz/docs/pc/serport.txt
 *              https://wiki.osdev.org/Serial_Ports
 */

// TODO : Implement RX, interrupts, {fifo,line,modem} controls

static void vmd_8250uart_putc(vm_t* vm, vmd_8250uart_t* uart, uint8_t read_byte) {
	if (uart->output_buffer_size >= sizeof(uart->output_buffer) - 1) {
		// We should have already flushed it
		uart->output_buffer_size = 0;
	}
	uart->output_buffer[uart->output_buffer_size] = read_byte;
	uart->output_buffer_size++;

	if (read_byte == '\n' || uart->output_buffer_size >= sizeof(uart->output_buffer) - 1) {
		if (uart->output_buffer_size > 0 && uart->output_buffer[uart->output_buffer_size - 1] == '\n') {
			uart->output_buffer[uart->output_buffer_size - 1] = 0;
		} else {
			uart->output_buffer[uart->output_buffer_size] = 0;
		}
		stdio_printf("\033[34;1mvm \"%s\"(%s):\033[0m %s\n", vm->name, uart->port_name, uart->output_buffer);
		uart->output_buffer_size = 0;
	}
}

static void vmd_8250uart_debug_print_control_regs(vm_t* vm, vmd_8250uart_t* uart) {
	unsigned int baud_rate = uart->baud_rate_divisor == 0 ? 0
							      : COM_BAUD_RATE_BASE / uart->baud_rate_divisor;
	LOG("\"%s\"(%s) : %ubps INT:%xpb FIFO:%xpb LINE:%xpb MODEM:%xpb",
	    vm->name, uart->port_name, baud_rate, uart->int_enable_reg,
	    uart->fifo_control_reg, uart->line_control_reg, uart->modem_control_reg);
}

static void vmd_8250uart_outb_handler(vm_t* vm, void* device_data, uint16_t port, uint8_t read_byte) {
	vmd_8250uart_t* uart = device_data;
	uint16_t port_offset = port - uart->port_base;
	bool print_debug = true;

	switch (port_offset) {
		case COM_DATA_REG:  // or COM_BAUD_LSB_REG
			if (uart->line_control_reg & COM_LINE_CONTROL_DLAB) {
				uart->baud_rate_divisor = (uart->baud_rate_divisor & 0xff00) | read_byte;
			} else {
				vmd_8250uart_putc(vm, uart, read_byte);
				print_debug = false;
			}
			break;
		case COM_INT_ENABLE_REG:  // or COM_BAUD_MSB_REG
			if (uart->line_control_reg & COM_LINE_CONTROL_DLAB) {
				uart->baud_rate_divisor = (uart->baud_rate_divisor & 0xff) | (read_byte << 8);
			} else {
				uart->int_enable_reg = read_byte & (COM_INT_ENABLE_DATA_AVAILABLE |
								    COM_INT_ENABLE_TRANSMIT_EMPTY |
								    COM_INT_ENABLE_BREAK_OR_ERROR |
								    COM_INT_ENABLE_STATUS_CHANGE);
			}
			break;
		case COM_FIFO_CONTROL_REG:
			uart->fifo_control_reg = read_byte & (COM_FIFO_CONTROL_ENABLE |
							      COM_FIFO_CONTROL_CLEAR_RX_FIFO |
							      COM_FIFO_CONTROL_CLEAR_TX_FIFO |
							      COM_FIFO_CONTROL_DMA_MODE |
							      COM_FIFO_CONTROL_TL_MASK);
			break;
		case COM_LINE_CONTROL_REG:
			uart->line_control_reg = read_byte;
			break;
		case COM_MODEM_CONTROL_REG:
			uart->modem_control_reg = read_byte & (COM_MODEM_CONTROL_DTR_PIN |
							       COM_MODEM_CONTROL_RTS_PIN |
							       COM_MODEM_CONTROL_IRQ |
							       COM_MODEM_CONTROL_LOOP);
			break;
		case COM_LINE_STATUS_REG:
		case COM_MODEM_STATUS_REG:
		case COM_SCRATCH_REG:
			// The status registers are read-only and the scratch register is ignored
			break;
		default:
			PANIC("Reached 8250uart outb handler with unexpected port");
	}

	if (print_debug) {
		vmd_8250uart_debug_print_control_regs(vm, uart);
	}
}

static uint8_t vmd_8250uart_inb_handler(vm_t* vm, void* device_data, uint16_t port) {
	(void)vm;
	vmd_8250uart_t* uart = device_data;
	uint16_t port_offset = port - uart->port_base;

	switch (port_offset) {
		case COM_DATA_REG:  // or COM_BAUD_LSB_REG
			if (uart->line_control_reg & COM_LINE_CONTROL_DLAB) {
				return uart->baud_rate_divisor & 0xff;
			} else {
				return 0;  // TODO : RX
			}
		case COM_INT_ENABLE_REG:  // or COM_BAUD_MSB_REG
			if (uart->line_control_reg & COM_LINE_CONTROL_DLAB) {
				return uart->baud_rate_divisor >> 8;
			} else {
				return uart->int_enable_reg;
			}
		case COM_INT_ID_REG:
			return 0;  // TODO : interrupts
		case COM_LINE_CONTROL_REG:
			return uart->line_control_reg;
		case COM_MODEM_CONTROL_REG:
			return uart->modem_control_reg;
		case COM_LINE_STATUS_REG:
			// We are always ready to transmit
			return COM_LINE_STATUS_TRANSMITTER_HOLDING_REGISTER_EMPTY |
			       COM_LINE_STATUS_TRANSMITTER_EMPTY;
		case COM_MODEM_STATUS_REG:
			return 0;
		case COM_SCRATCH_REG:
			return 0;
		default:
			PANIC("Reached 8250uart inb handler with unexpected port");
	}
}

vm_device_t* vmd_8250uart_create(uint16_t port_base, const char* port_name) {
	vm_device_t* device = arena_allocate(vmd_get_device_arena(), sizeof(vm_device_t));
	memset(device, 0, sizeof(*device));
	vmd_8250uart_t* uart = arena_allocate(vmd_get_device_arena(), sizeof(vmd_8250uart_t));
	memset(uart, 0, sizeof(*uart));

	uart->port_name = port_name;
	uart->port_base = port_base;

	static_assert(sizeof(device->ports) / sizeof(device->ports[0]) >= COM_PORTS_LEN,
		      "Not enough ports avaliable in vm_device_t for vmd_8250uart");
	for (size_t i = 0; i < COM_PORTS_LEN; i++) {
		device->ports[i] = port_base + i;
	}
	device->ports_len = COM_PORTS_LEN;
	device->device_data = uart;
	device->outb_handler = vmd_8250uart_outb_handler;
	device->inb_handler = vmd_8250uart_inb_handler;

	return device;
}
