#include <stdbool.h>
#include <stdint.h>

#include <rkhv/io_ports.h>

#include "serial.h"

static bool serial_initialized = false;

static void serial_init(void) {
	outb(COM1_INT_ENABLE_REG, 0);
	outb(COM1_LINE_CONTROL_REG, COM_LINE_CONTROL_DLAB);
	// Set baud divisor to 1 : 115200 baud
	outb(COM1_BAUD_LSB_REG, 1);
	outb(COM1_BAUD_MSB_REG, 0);
	outb(COM1_LINE_CONTROL_REG, COM_LINE_CONTROL_8BITS | COM_LINE_CONTROL_PARITY_NONE);
	serial_initialized = true;
	/* NOTE : We should probably check for the presence of COM1 using the loopback feature but we wouldn't have
	 *        any stdio otherwise so we'll just asume it exists for now
	 */
}

void serial_write_byte(uint8_t c) {
	if (!serial_initialized) {
		serial_init();
	}

	while (!(inb(COM1_LINE_STATUS_REG) & COM_LINE_STATUS_TRANSMITTER_HOLDING_REGISTER_EMPTY))
		;
	outb(COM1_DATA_REG, c);
}
