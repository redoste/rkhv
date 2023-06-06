#include <stddef.h>

#include <rkhv/mem.h>

#define LOG_CATEGORY "vm"
#include <rkhv/arena.h>
#include <rkhv/panic.h>
#include <rkhv/stdio.h>

#include "vm_manager.h"
#include "vmx_instructions.h"

static arena_t* vm_manager_arena = NULL;
vm_t* vm_manager_vm_list = NULL;

vm_t* vm_manager_create_vmx_machine(const char* name) {
	if (!vm_manager_arena) {
		vm_manager_arena = arena_create();
	}

	vm_t* result = (vm_t*)arena_allocate(vm_manager_arena, sizeof(vm_t));
	memset(result, 0, sizeof(vm_t));
	result->next = vm_manager_vm_list;
	vm_manager_vm_list = result;

	result->name = name;
	return result;
}

__attribute__((noreturn)) void vm_manager_launch(vm_t* vm) {
	LOG("vmptrld-ing  \"%s\" VMCS", vm->name);
	VMX_ASSERT(vmx_vmptrld(vm->vmcs_region));
	LOG("vmlaunch-ing \"%s\"", vm->name);
	VMX_ASSERT(vmx_vmlaunch());
	PANIC("vm_launch reached its end");
}
