#include <rkhv/chainload.h>

#define LOG_CATEGORY "main"
#include "stdio.h"

#include "interrupts.h"
#include "panic.h"
#include "segments.h"

__attribute__((section(".text.entry"))) void hvmain(chainload_page_t* chainload_page) {
	LOG("rkhv loaded - git:" RKHV_HASH " - chainload_page @ %p", (void*)chainload_page);
	segments_setup();
	interrupts_setup();

	PANIC("hvmain reached its end");
}
