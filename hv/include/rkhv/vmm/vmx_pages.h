#ifndef COMMON_VMM_VMX_PAGES_H
#define COMMON_VMM_VMX_PAGES_H

#include <rkhv/stdint.h>

#define VMX_MAX_EPT_PAGES 16

uintptr_t vmx_get_vmxon_region(void);
uintptr_t vmx_get_free_vmcs_region(void);
uint64_t* vmx_get_free_ept_page(void);

#endif
