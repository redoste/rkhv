#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include <stdarg.h>

#include <rkhv/stdint.h>

char* str_itoh(char* out_string, uintptr_t value, size_t size);
char* str_utoa(char* buffer, uintptr_t value, size_t buffer_size);

/* This is a custom printf implementation that doesn't use any memory allocation, thus it's usable before the memory
 * management is initialized. Even if the format strings look like the standard libc ones, this is a different language
 *
 * %%            : Percent
 * %s            : String
 * %p            : 64 bits pointer
 * %u            : 64 bits unsigned integer
 * %x[put][bwdq] : 0 left-padded hexadecimal unsigned integer
 *                 p : prefixed by 0x          b : byte (uint8_t)
 *                 u : unprefixed (default)    w : word (uint16_t)
 *                 t : zeros left-trimmed      d : dword (uint32_t)
 *                                             q : qword (uint64_t) (default)
 * %z[format]    : Treated as [format], for printf warnings compatibility
 */
typedef void str_printf_char_handler(char);
typedef void str_printf_string_handler(const char*);
void str_printf_core(const char* fmt,
		     va_list args,
		     str_printf_char_handler char_handler,
		     str_printf_string_handler string_handler);

size_t str_strlen(const char* s);

#endif
