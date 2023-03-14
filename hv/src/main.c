#include <rkhv/chainload.h>

#define LOG_CATEGORY "main"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>

#include "interrupts.h"
#include "memory_management/memory_management.h"
#include "segments.h"
#include "vmm/vmx_init.h"
#include "vmm/vmx_instructions.h"
#include "vmm/vmx_vmcs.h"

__attribute__((section(".text.entry")))
__attribute__((noreturn)) void
hvmain(chainload_page_t* chainload_page) {
	LOG("rkhv loaded - git:" RKHV_HASH " - chainload_page @ %p", (void*)chainload_page);
	segments_setup();
	interrupts_setup();
	mm_setup(chainload_page);

	vmx_setup();

	uintptr_t vmcs_region = vmx_create_initialized_vmcs();
	LOG("new initialized VMCS created @ %p", (void*)vmcs_region);
	VMX_ASSERT(vmx_vmptrld(vmcs_region));
	LOG("VMCS now active and current, about to vmlaunch");
	VMX_ASSERT(vmx_vmlaunch());

	PANIC("hvmain reached its end");
}
