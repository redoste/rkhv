#include <stdbool.h>
#include <stddef.h>

#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#include <rkhv/vmm/vm_manager.h>

#include "vmm_vm_manager.h"

bool mm_page_used_by_vm_manager(uintptr_t page_physical_address) {
	for (vm_t* iter = vm_manager_vm_list; iter != NULL; iter = iter->next) {
		if (iter->flags & VM_T_FLAG_VMCS_INITIALIZED &&
		    iter->vmcs_region == page_physical_address) {
			return true;
		}

		for (vm_page_list_t* pl_iter = iter->tracked_pages; pl_iter != NULL; pl_iter = pl_iter->next) {
			uintptr_t list_first_page = pl_iter->physical_address;
			uintptr_t list_last_page = list_first_page + ((pl_iter->pages - 1) * PAGE_SIZE);
			if (page_physical_address >= list_first_page && page_physical_address <= list_last_page) {
				return true;
			}
		}
	}
	return false;
}
