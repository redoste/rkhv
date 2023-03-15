#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <rkhv/stdint.h>

#include "string.h"

char* str_itoh(char* out_string, uintptr_t value, size_t size) {
	char* trimmed_start = NULL;
	for (intptr_t index = size - 1; index >= 0; index--) {
		uint8_t nibble = value & 0xf;
		out_string[index] = nibble < 10 ? (u'0' + nibble) : (u'a' + nibble - 10);
		value = value >> 4;
		if (value == 0 && trimmed_start == NULL) {
			trimmed_start = &out_string[index];
		}
	}
	return trimmed_start;
}

char* str_utoa(char* buffer, uintptr_t value, size_t buffer_size) {
	size_t index = buffer_size - 1;
	do {
		buffer[index] = '0' + (value % 10);
		value /= 10;
		if (index == 0) {
			break;
		}
		if (value != 0) {
			index--;
		}
	} while (value != 0);
	return buffer + index;
}

void str_printf_core(const char* fmt,
		     va_list args,
		     str_printf_char_handler char_handler,
		     str_printf_string_handler string_handler) {
	bool read_z = false;
	while (*fmt) {
		if (*fmt != '%' && !read_z) {
			char_handler(*fmt);
			fmt++;
			continue;
		}

		if (!read_z) {
			fmt++;
		}
		read_z = false;

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
			case 'u': {
				char buffer[32];
				uintptr_t value = va_arg(args, uintptr_t);
				char* start = str_utoa(buffer, value, sizeof(buffer) - 1);
				buffer[sizeof(buffer) - 1] = 0;
				string_handler(start);
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
				char* trimmed_start = str_itoh(&buffer[2], value, len);
				char* start = prefixed_char == 't'   ? trimmed_start
					      : prefixed_char == 'p' ? buffer
								     : &buffer[2];
				string_handler(start);
				break;
			}
			case 'z': {
				read_z = true;
				break;
			}
			default: {
				char_handler(type);
				break;
			}
		}
	}
}
