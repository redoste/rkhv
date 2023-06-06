#include <stdbool.h>
#include <stddef.h>

#include <rkhv/stdint.h>

#include <rkhv/vmm/vm_manager.h>

#include "vmm_vm_manager.h"

bool mm_page_used_by_vm_manager(uintptr_t page_physical_address) {
	for (vm_t* iter = vm_manager_vm_list; iter != NULL; iter = iter->next) {
		if (iter->flags & VM_T_FLAG_VMCS_INITIALIZED &&
		    iter->vmcs_region == page_physical_address) {
			return true;
		}
	}
	return false;
}
