#ifndef STRING_H
#define STRING_H

#include <efi.h>

void str_itouh(CHAR16* out_string, uintptr_t value, size_t out_len);  // itouh : int to unicode hex

#endif
