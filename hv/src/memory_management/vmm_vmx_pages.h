#ifndef VMM_VMX_PAGES_H
#define VMM_VMX_PAGES_H

#include <stdbool.h>

#include <rkhv/stdint.h>

bool mm_page_used_by_vmx(uintptr_t page_physical_address);

#endif
