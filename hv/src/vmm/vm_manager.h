#ifndef VM_MANAGER
#define VM_MANAGER

#include <rkhv/vmm/vm_manager.h>

__attribute__((noreturn)) void vm_manager_launch(vm_t* vm);

void vm_manager_track_ept_page(vm_t* vm, const uint64_t* ept_page);

#endif
