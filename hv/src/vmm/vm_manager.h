#ifndef VM_MANAGER
#define VM_MANAGER

#include <rkhv/vmm/vm_manager.h>

__attribute__((noreturn)) void vm_manager_launch(vm_t* vm);
vm_t* vm_manager_get_current_vm(void);

void vm_manager_track_ept_page(vm_t* vm, const uint64_t* ept_page);
void vm_manager_allocate_guest_physical_memory(vm_t* vm, size_t guest_physical_pages);

void vm_manager_register_device(vm_t* vm, vm_device_t* device);

#endif
