#include <rkhv/memory_map.h>
#include <rkhv/stdint.h>

#include <rkhv/vmm/vmx_pages.h>

#include "vm_manager.h"
#include "vmx_ept.h"

void vmx_ept_create_identity_mapping(vm_t* vm) {
	uint64_t* ept_pdpt = vmx_allocate_ept_page();
	for (size_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
		ept_pdpt[i] = EPT_PDPTE_READ | EPT_PDPTE_WRITE | EPT_PDPTE_EXECUTE | EPT_PDPTE_PAGE_SIZE |
			      ((i << GUEST_PHYSICAL_ADDRESS_DP_SHIFT) & EPT_PDPTE_PHYSICAL_ADDRESS_PAGE);
	}

	uint64_t* ept_pml4 = vmx_allocate_ept_page();
	ept_pml4[0] = EPT_PML4E_READ | EPT_PML4E_WRITE | EPT_PML4E_EXECUTE |
		      (V2P_IDENTITY_MAP(ept_pdpt) & EPT_PML4E_PHYSICAL_ADDRESS_PDPT);

	vm->vmcs_config.eptp = V2P_IDENTITY_MAP(ept_pml4);
}
