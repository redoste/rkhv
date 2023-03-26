#ifndef COMMON_VMM_VMX_PAGES_H
#define COMMON_VMM_VMX_PAGES_H

#include <rkhv/stdint.h>

#define VMX_MAX_VMCS      2
#define VMX_MAX_EPT_PAGES 16

uintptr_t vmx_get_vmxon_region(void);
uintptr_t vmx_get_new_vmcs_region(void);
uint64_t* vmx_allocate_ept_page(void);

#endif
