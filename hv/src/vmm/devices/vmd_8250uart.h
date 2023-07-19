#ifndef VMD_8250UART_H
#define VMD_8250UART_H

#include <rkhv/stdint.h>

#include <rkhv/vmm/vm_manager.h>

vm_device_t* vmd_8250uart_create(uint16_t port_base, const char* port_name);

#define VMD_8250UART_T_OUTPUT_BUFFER_CAPACITY 128
typedef struct vmd_8250uart_t {
	uint8_t output_buffer[VMD_8250UART_T_OUTPUT_BUFFER_CAPACITY];
	size_t output_buffer_size;
	const char* port_name;
	uint16_t port_base;

	uint16_t baud_rate_divisor;
	uint8_t int_enable_reg;
	uint8_t fifo_control_reg;
	uint8_t line_control_reg;
	uint8_t modem_control_reg;
} vmd_8250uart_t;

#endif
