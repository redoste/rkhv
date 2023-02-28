#ifndef COMMON_VMM_VMX_PAGES_H
#define COMMON_VMM_VMX_PAGES_H

#include <rkhv/stdint.h>

#define VMX_MAX_VMCS 2

uintptr_t vmx_get_vmxon_region(void);
uintptr_t vmx_get_new_vmcs_region(void);

#endif
