#ifndef STRING_H
#define STRING_H

#include <stdarg.h>

#include <rkhv/stdint.h>

void str_itoh(char* out_string, uintptr_t value, size_t size);

/* This is a custom printf implementation that doesn't use any memory allocation, thus it's usable before the memory
 * management is initialized. Even if the format strings look like the standard libc ones, this is a different language
 *
 * %%           : Percent
 * %s           : String
 * %p           : 64 bits pointer
 * %x[pu][bwdq] : 0 left-padded hexadecimal unsigned integer
 *                p : prefixed by 0x          b : byte (uint8_t)
 *                u : unprefixed (default)    w : word (uint16_t)
 *                                            d : dword (uint32_t)
 *                                            q : qword (uint64_t) (default)
 * %z[format]   : Treated as [format], for printf warnings compatibility
 */
typedef void str_printf_char_handler(char);
typedef void str_printf_string_handler(const char*);
void str_printf_core(const char* fmt,
		     va_list args,
		     str_printf_char_handler char_handler,
		     str_printf_string_handler string_handler);

#endif
