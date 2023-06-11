#ifndef VM_GUEST_PAGING
#define VM_GUEST_PAGING

#include <stdbool.h>

#include <rkhv/stdint.h>

#include <rkhv/vmm/vm_manager.h>

#include "vmx_vmexit.h"

uintptr_t vm_guest_paging_setup_identity(vm_t* vm);
bool vm_guest_paging_get_guest_physical_address(vm_t* vm, uintptr_t guest_cr3, uintptr_t guest_linear_addr, uintptr_t* guest_physical_address);
void* vm_guest_paging_get_host_virtual_address_during_vmexit(vmx_vmexit_state_t* vm_state, uintptr_t guest_virtual_address, size_t guest_data_size);

#endif
