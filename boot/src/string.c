#include <efi.h>

#include "string.h"

void str_itouh(CHAR16* out_string, uintptr_t value, size_t out_len) {
	for (intptr_t index = out_len - 1; index >= 0; index--) {
		uint8_t nibble = value & 0xf;
		out_string[index] = nibble < 10 ? (u'0' + nibble) : (u'a' + nibble - 10);
		value = value >> 4;
	}
}
