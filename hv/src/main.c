#include <rkhv/chainload.h>

#include "stdio.h"

__attribute__((section(".text.entry"))) void hvmain(chainload_page_t* chainload_page) {
	// TODO : setup gdt
	// TODO : setup idt
	stdio_puts("rkhv - git:" RKHV_HASH "\n");
	(void)chainload_page;

	while (1) {
		asm("hlt");
	}
}
