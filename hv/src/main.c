#include <rkhv/chainload.h>
#include <rkhv/cr_msr.h>
#define COMMON_MEM_IMPLEMENTATION
#include <rkhv/mem.h>

#define LOG_CATEGORY "main"
#include <rkhv/interrupts.h>
#include <rkhv/panic.h>
#include <rkhv/segments.h>
#include <rkhv/stdio.h>

#include "interrupts.h"
#include "memory_management/memory_management.h"
#include "segments.h"
#include "vmm/vm_manager.h"
#include "vmm/vmx_ept.h"
#include "vmm/vmx_init.h"
#include "vmm/vmx_vmcs.h"

extern void vm_guest_hello_world(void);

__attribute__((section(".text.entry")))
__attribute__((noreturn)) void
hvmain(chainload_page_t* chainload_page) {
	LOG("rkhv loaded - git:" RKHV_HASH " - chainload_page @ %p", (void*)chainload_page);
	segments_setup();
	interrupts_setup();
	mm_setup(chainload_page);

	vmx_setup();

	vm_t* vm = vm_manager_create_vmx_machine("hello_vm");

	vmx_ept_create_identity_mapping(vm);
	LOG("new identity mapped EPT created @ %p", (void*)vm->vmcs_config.eptp);

	vm->vmcs_config.guest_state.rip = (uintptr_t)&vm_guest_hello_world;
	vm->vmcs_config.guest_state.rsp = 0;
	vm->vmcs_config.guest_state.cr0 = cr0_read();
	vm->vmcs_config.guest_state.cr3 = cr3_read();
	vm->vmcs_config.guest_state.cr4 = cr4_read();
	vm->vmcs_config.guest_state.cs = RKHV_CS;
	vm->vmcs_config.guest_state.ds = RKHV_DS;
	interrupts_sidt(&vm->vmcs_config.guest_state.idtr);
	segments_sgdt(&vm->vmcs_config.guest_state.gdtr);

	vmx_create_initialized_vmcs(vm);
	LOG("new initialized VMCS created @ %p", (void*)vm->vmcs_region);

	vm_manager_launch(vm);
}
