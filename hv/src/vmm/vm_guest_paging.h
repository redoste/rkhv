#ifndef VM_GUEST_PAGING
#define VM_GUEST_PAGING

#include <rkhv/stdint.h>

#include <rkhv/vmm/vm_manager.h>

uintptr_t vm_guest_paging_setup_identity(vm_t* vm);

#endif
