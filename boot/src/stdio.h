#ifndef STDIO_H
#define STDIO_H

#include <efi.h>

EFI_STATUS stdio_puts(const CHAR16* str);
EFI_STATUS stdio_print_banner(void);
EFI_STATUS stdio_read_key(EFI_INPUT_KEY* key);
EFI_STATUS stdio_perror(const CHAR16* location, EFI_STATUS status);

#endif
