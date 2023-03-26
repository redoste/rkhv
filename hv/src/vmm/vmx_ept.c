#include <rkhv/memory_map.h>
#include <rkhv/stdint.h>

#include <rkhv/vmm/vmx_pages.h>

#include "vmx_ept.h"

uintptr_t vmx_ept_create_transparent(void) {
	uint64_t* ept_pdpt = vmx_allocate_ept_page();
	for (size_t i = 0; i < PAGE_SIZE / sizeof(uint64_t); i++) {
		ept_pdpt[i] = EPT_PDPTE_READ | EPT_PDPTE_WRITE | EPT_PDPTE_EXECUTE | EPT_PDPTE_PAGE_SIZE |
			      ((i << GUEST_PHYSICAL_ADDRESS_DP_SHIFT) & EPT_PDPTE_PHYSICAL_ADDRESS_PAGE);
	}

	uint64_t* ept_pml4 = vmx_allocate_ept_page();
	for (size_t i = 1; i < PAGE_SIZE / sizeof(uint64_t); i++) {
		ept_pml4[i] = 0;
	}
	ept_pml4[0] = EPT_PML4E_READ | EPT_PML4E_WRITE | EPT_PML4E_EXECUTE |
		      (V2P_IDENTITY_MAP(ept_pdpt) & EPT_PML4E_PHYSICAL_ADDRESS_PDPT);

	return V2P_IDENTITY_MAP(ept_pml4);
}
