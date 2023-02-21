#include <stdarg.h>

#include <rkhv/stdint.h>

#include "string.h"

void str_itoh(char* out_string, uintptr_t value, size_t size) {
	for (intptr_t index = size - 1; index >= 0; index--) {
		uint8_t nibble = value & 0xf;
		out_string[index] = nibble < 10 ? (u'0' + nibble) : (u'a' + nibble - 10);
		value = value >> 4;
	}
}

void str_printf_core(const char* fmt,
		     va_list args,
		     str_printf_char_handler char_handler,
		     str_printf_string_handler string_handler) {
	while (*fmt) {
		if (*fmt != '%') {
			char_handler(*fmt);
			fmt++;
			continue;
		}

		fmt++;
		char type = *fmt;
		fmt++;

		switch (type) {
			case 's': {
				char* substr = va_arg(args, char*);
				string_handler(substr);
				break;
			}
			case 'p': {
				char buffer[] = "0x????????????????";
				uintptr_t value = va_arg(args, uintptr_t);
				str_itoh(&buffer[2], value, 16);
				string_handler(buffer);
				break;
			}
			case 'x': {
				char buffer[] = "0x????????????????";
				uintptr_t value = va_arg(args, uintptr_t);

				char prefixed_char = *(fmt++);
				char len_char = *(fmt++);
				size_t len = len_char == 'b'   ? 2
					     : len_char == 'w' ? 4
					     : len_char == 'd' ? 8
					     : len_char == 'q' ? 16
							       : 16;

				buffer[len + 2] = 0;
				str_itoh(&buffer[2], value, len);
				char* start = prefixed_char == 'p' ? buffer : &buffer[2];
				string_handler(start);
				break;
			}
			default: {
				char_handler(type);
				break;
			}
		}
	}
}
