#include <efi.h>

#include "main.h"
#include "stdio.h"
#include "string.h"

static const CHAR16 STDIO_BANNER[] =
	u"################################################################################\r\n"
	u"#                                                                              #\r\n"
	u"#                     rkhv - EFI bootloader - git:" RKHV_HASH
	"                      #\r\n"
	u"#                                                                              #\r\n"
	u"################################################################################\r\n";

EFI_STATUS stdio_puts(const CHAR16* str) {
	// We drop the const because there are no reasons the EFI would want to write to our buffer anyway
	return EFI_ST->ConOut->OutputString(EFI_ST->ConOut, (CHAR16*)str);
}

EFI_STATUS stdio_print_banner(void) {
	RETURN_EFI(EFI_ST->ConOut->ClearScreen(EFI_ST->ConOut));
	RETURN_EFI(stdio_puts(STDIO_BANNER));
	return EFI_SUCCESS;
}

EFI_STATUS stdio_read_key(EFI_INPUT_KEY* key) {
	RETURN_EFI(EFI_ST->BootServices->WaitForEvent(1, &EFI_ST->ConIn->WaitForKey, NULL));
	RETURN_EFI(EFI_ST->ConIn->ReadKeyStroke(EFI_ST->ConIn, key));
	return EFI_SUCCESS;
}

EFI_STATUS stdio_perror(const CHAR16* location, EFI_STATUS status) {
	CHAR16 buffer[] = u": EFI_ERR(0xxx)\r\n";
	uint8_t error = ~EFI_ERROR_MASK & status & 0xff;
	str_itouh(&buffer[12], error, 2);

	RETURN_EFI(stdio_puts(location));
	RETURN_EFI(stdio_puts(buffer));
	return EFI_SUCCESS;
}
