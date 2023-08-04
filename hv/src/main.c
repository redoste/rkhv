#include <stddef.h>

#include <rkhv/chainload.h>
#include <rkhv/cr_msr.h>
#define COMMON_MEM_IMPLEMENTATION
#include <rkhv/mem.h>
#include <rkhv/memory_map.h>

#define LOG_CATEGORY "main"
#include <rkhv/interrupts.h>
#include <rkhv/panic.h>
#include <rkhv/segments.h>
#include <rkhv/serial.h>
#include <rkhv/stdio.h>

#include "interrupts.h"
#include "memory_management/memory_management.h"
#include "segments.h"
#include "vmm/devices/vmd_8250uart.h"
#include "vmm/guest_linux_loader.h"
#include "vmm/vm_guest_paging.h"
#include "vmm/vm_manager.h"
#include "vmm/vmx_ept.h"
#include "vmm/vmx_init.h"
#include "vmm/vmx_vmcs.h"
#include "xsave.h"

extern void vm_guest_hello_world(void);
extern size_t vm_guest_hello_world_size;

__attribute__((section(".text.entry")))
__attribute__((noreturn)) void
hvmain(chainload_page_t* chainload_page) {
	LOG("rkhv loaded - git:" RKHV_HASH " - chainload_page @ %p", (void*)chainload_page);
	segments_setup();
	xsave_setup();
	interrupts_setup();
	mm_setup(chainload_page);

	vmx_setup();

	vm_t* vm = vm_manager_create_vmx_machine("hello_vm");

	vm_manager_allocate_guest_physical_memory(vm, (64 * 1024 * 1024) / PAGE_SIZE);
	LOG("64MiB guest physical memory allocated for VM with EPT @ %p", (void*)vm->vmcs_config.eptp);

	size_t linux_bzimage_size;
	uint8_t* linux_bzimage = mm_get_attachment(CHAINLOAD_ATTACHMENT_LINUX_BZIMAGE, &linux_bzimage_size);
	if (linux_bzimage != NULL && linux_bzimage_size > 0) {
		const char* cmdline_default = "earlyprintk=serial,ttyS0 console=ttyS0 nokaslr noapic nolapic";
		vm_guest_linux_loader(vm, linux_bzimage, linux_bzimage_size, cmdline_default);
	} else {
		uintptr_t guest_cr3 = vm_guest_paging_setup_identity(vm);
		LOG("guest identity paging setup with PML4 @ %p", (void*)guest_cr3);

		uintptr_t guest_rip = 0x7c00;
		uintptr_t guest_rip_hostpa;
		if (!vmx_ept_get_host_physical_address(vm, guest_rip, &guest_rip_hostpa)) {
			PANIC("Unable to resolve guest physical address for copying guest code");
		}
		memcpy(P2V_IDENTITY_MAP(guest_rip_hostpa), (void*)vm_guest_hello_world, vm_guest_hello_world_size);

		vm->vmcs_config.guest_state.rip = guest_rip;
		vm->vmcs_config.guest_state.rsp = 0;
		vm->vmcs_config.guest_state.cr0 = cr0_read();
		vm->vmcs_config.guest_state.cr3 = guest_cr3;
		vm->vmcs_config.guest_state.cr4 = cr4_read();
		vm->vmcs_config.guest_state.ia32_efer = rdmsr(IA32_EFER);
		vm->vmcs_config.guest_state.cs = RKHV_CS;
		vm->vmcs_config.guest_state.ds = RKHV_DS;
		interrupts_sidt(&vm->vmcs_config.guest_state.idtr);
		segments_sgdt(&vm->vmcs_config.guest_state.gdtr);
	}

	vmx_create_initialized_vmcs(vm);
	LOG("new initialized VMCS created @ %p", (void*)vm->vmcs_region);

	vm_manager_register_device(vm, vmd_8250uart_create(COM1, "COM1"));
	vm_manager_register_device(vm, vmd_8250uart_create(COM2, "COM2"));

	vm_manager_launch(vm);
}
