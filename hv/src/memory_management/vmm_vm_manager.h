#ifndef MM_VMM_VM_MANAGER
#define MM_VMM_VM_MANAGER

#include <stdbool.h>

#include <rkhv/stdint.h>

bool mm_page_used_by_vm_manager(uintptr_t page_physical_address);

#endif
