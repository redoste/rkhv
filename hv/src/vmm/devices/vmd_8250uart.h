#ifndef VMD_8250UART_H
#define VMD_8250UART_H

#include <rkhv/vmm/vm_manager.h>

vm_device_t* vmd_8250uart_create(void);

#define VMD_8250UART_T_OUTPUT_BUFFER_CAPACITY 128
typedef struct vmd_8250uart_t {
	uint8_t output_buffer[VMD_8250UART_T_OUTPUT_BUFFER_CAPACITY];
	size_t output_buffer_size;
} vmd_8250uart_t;

#endif
