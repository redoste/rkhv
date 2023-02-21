#include <rkhv/chainload.h>

#define LOG_CATEGORY "main"
#include "stdio.h"

#include "segments.h"

__attribute__((section(".text.entry"))) void hvmain(chainload_page_t* chainload_page) {
	// TODO : setup idt
	LOG("rkhv loaded - git:" RKHV_HASH " - chainload_page @ %p", (void*)chainload_page);
	segments_setup();

	while (1) {
		asm("hlt");
	}
}
