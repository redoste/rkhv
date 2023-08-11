#include <stddef.h>

#include <rkhv/mem.h>
#include <rkhv/memory_map.h>
#include <rkhv/paging.h>

#define LOG_CATEGORY "vm"
#include <rkhv/arena.h>
#include <rkhv/panic.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vmx_pages.h>

#include "vm_manager.h"
#include "vmx_ept.h"
#include "vmx_instructions.h"
#include "vmx_msr.h"

static arena_t* vm_manager_arena = NULL;
vm_t* vm_manager_vm_list = NULL;
static vm_t* vm_manager_current_vm = NULL;

vm_t* vm_manager_create_vmx_machine(const char* name) {
	if (!vm_manager_arena) {
		vm_manager_arena = arena_create();
	}

	vm_t* result = (vm_t*)arena_allocate(vm_manager_arena, sizeof(vm_t));
	memset(result, 0, sizeof(vm_t));
	result->next = vm_manager_vm_list;
	vm_manager_vm_list = result;

	result->name = name;
	/* TODO : This might not the be the best function for setting the inital value of XCR0
	 *        It doesn't matter as for now this value shouldn't change
	 *        Maybe this will be moved to a more complex initialization routine of the
	 *        permanent_state if the structure gets bigger
	 */
	result->permanent_state.xcr0 = 1;
	return result;
}

__attribute__((noreturn)) void vm_manager_launch(vm_t* vm) {
	vmx_msr_vmlaunch();

	LOG("vmptrld-ing  \"%s\" VMCS", vm->name);
	VMX_ASSERT(vmx_vmptrld(vm->vmcs_region));
	vm_manager_current_vm = vm;
	LOG("vmlaunch-ing \"%s\"", vm->name);
	VMX_ASSERT(vmx_vmlaunch(&vm->initial_gpr_state, &vm->permanent_state));

	PANIC("vm_launch reached its end");
}

vm_t* vm_manager_get_current_vm(void) {
	uintptr_t curr_vmcs_region;
	VMX_ASSERT(vmx_vmptrst(&curr_vmcs_region));
	if (vm_manager_current_vm == NULL ||
	    curr_vmcs_region != vm_manager_current_vm->vmcs_region) {
		PANIC("vm_manager_current_vm and current VMCS pointer unsynced");
	}
	return vm_manager_current_vm;
}

static void vm_manager_track_page(vm_t* vm, uintptr_t physical_address, vm_page_type_t page_type, bool reserved_pool) {
	vm_page_list_t* new_list = NULL;
	for (vm_page_list_t* iter = vm->tracked_pages; iter != NULL; iter = iter->next) {
		if (iter->pages == 0) {
			if (new_list == NULL) {
				new_list = iter;
			}
			continue;
		}
		if (iter->page_type != page_type || iter->reserved_pool != reserved_pool) {
			continue;
		}

		uintptr_t first_page = iter->physical_address;
		uintptr_t last_page = first_page + ((iter->pages - 1) * PAGE_SIZE);
		if (physical_address >= first_page && physical_address <= last_page) {
			// The page is already present in the current vm_page_list_t
			return;
		} else if (physical_address + PAGE_SIZE == first_page) {
			// Before the current vm_page_list_t : we move its start and extend it
			iter->physical_address = physical_address;
			iter->pages++;
			return;
		} else if (physical_address - PAGE_SIZE == last_page) {
			// After the current vm_page_list_t : we just extend it to one more page
			iter->pages++;
			return;
		}
	}

	if (new_list == NULL) {
		new_list = (vm_page_list_t*)arena_allocate(vm_manager_arena, sizeof(vm_page_list_t));
		new_list->next = vm->tracked_pages;
		vm->tracked_pages = new_list;
	}
	new_list->physical_address = physical_address;
	new_list->pages = 1;
	new_list->page_type = page_type;
	new_list->reserved_pool = reserved_pool;
}

static bool vm_manager_untrack_page_from_reserved_pool(vm_t* vm, uintptr_t* physical_address, vm_page_type_t page_type) {
	for (vm_page_list_t* iter = vm->tracked_pages; iter != NULL; iter = iter->next) {
		if (iter->page_type != page_type || !iter->reserved_pool || iter->pages == 0) {
			/* vm_page_list_t with 0 pages remaining will be repurposed by vm_manager_track_page
			 * since we can't free them anyway
			 */
			continue;
		}

		*physical_address = iter->physical_address + ((iter->pages - 1) * PAGE_SIZE);
		iter->pages--;
		return true;
	}
	return false;
}

void vm_manager_track_ept_page(vm_t* vm, const uint64_t* ept_page) {
	vm_manager_track_page(vm, V2P_IDENTITY_MAP(ept_page), VM_PAGE_TYPE_EPT, false);
}

bool vm_manager_untrack_ept_page_from_reserved_pool(vm_t* vm, uint64_t** ept_page) {
	uintptr_t physical_address;
	bool ret = vm_manager_untrack_page_from_reserved_pool(vm, &physical_address, VM_PAGE_TYPE_EPT);
	if (ret) {
		*ept_page = P2V_IDENTITY_MAP(physical_address);
	}
	return ret;
}

void vm_manager_allocate_guest_physical_memory(vm_t* vm, size_t guest_physical_pages) {
	if (vm->guest_physical_pages != 0) {
		PANIC("Reallocating guest physical memory on a VM with memory already allocated");
	}
	vm->guest_physical_pages = guest_physical_pages;

#define CEIL_DIV(a, b) (((a) / (b)) + ((a) % (b) == 0 ? 0 : 1))
	size_t ept_amount_pt = CEIL_DIV(guest_physical_pages, PAGE_TABLE_ENTRIES);
	size_t ept_amount_pd = CEIL_DIV(ept_amount_pt, PAGE_TABLE_ENTRIES);
	size_t ept_amount_pdpt = CEIL_DIV(ept_amount_pd, PAGE_TABLE_ENTRIES);
	size_t ept_amount_pml4 = CEIL_DIV(ept_amount_pdpt, PAGE_TABLE_ENTRIES);
	size_t ept_pages = ept_amount_pt + ept_amount_pd + ept_amount_pdpt + ept_amount_pml4;
#undef CEIL_DIV
	for (size_t i = 0; i < ept_pages; i++) {
		uintptr_t ept_page_pa = V2P_IDENTITY_MAP(vmx_get_free_ept_page());
		vm_manager_track_page(vm, ept_page_pa, VM_PAGE_TYPE_EPT, true);
	}

	for (size_t i = 0; i < guest_physical_pages; i++) {
		uintptr_t page_pa = vm_manager_get_free_guest_physical_page();
		vm_manager_track_page(vm, page_pa, VM_PAGE_TYPE_GUEST_PHYSICAL, false);
		vmx_ept_map_page(vm, i * PAGE_SIZE, page_pa);
	}
}

void vm_manager_register_device(vm_t* vm, vm_device_t* device) {
	if (device->next != NULL) {
		PANIC("Registering a device that is probably already registered");
	}

	device->next = vm->devices;
	vm->devices = device;
}
