#include <rkhv/chainload.h>

#define LOG_CATEGORY "main"
#include "stdio.h"

__attribute__((section(".text.entry"))) void hvmain(chainload_page_t* chainload_page) {
	// TODO : setup gdt
	// TODO : setup idt
	LOG("rkhv loaded - git:" RKHV_HASH " - chainload_page @ %p", (void*)chainload_page);

	while (1) {
		asm("hlt");
	}
}
