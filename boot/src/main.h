#ifndef MAIN_H
#define MAIN_H

#include <efi.h>

#include <rkhv/chainload.h>

#define SECTION_PAGES                    (0x100000 / EFI_PAGE_SIZE)  // 1 MiB
#define STACK_PAGES                      (0x100000 / EFI_PAGE_SIZE)
#define PAGE_TABLE_POOL_DEFAULT_CAPACITY RKHV_MAX_PAGE_TABLE_PAGES

extern EFI_SYSTEM_TABLE* EFI_ST;

#define AS_STR(x)  #x
#define XAS_STR(x) AS_STR(x)

#define VERIFY_EFI(expr)                                                                             \
	do {                                                                                         \
		EFI_STATUS ret = (expr);                                                             \
		if (EFI_ERROR(ret)) {                                                                \
			/* TODO : find a panic fallback when stdio_perror or stdio_read_key fails */ \
			stdio_perror(u"" __FILE__ ":" XAS_STR(__LINE__), ret);                       \
			stdio_read_key(NULL);                                                        \
			return ret;                                                                  \
		}                                                                                    \
	} while (0)

#define RETURN_EFI(expr)                 \
	do {                             \
		EFI_STATUS ret = (expr); \
		if (EFI_ERROR(ret)) {    \
			return ret;      \
		}                        \
	} while (0)

#endif
