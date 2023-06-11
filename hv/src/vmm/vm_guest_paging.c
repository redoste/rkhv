#include <rkhv/mem.h>
#include <rkhv/memory_map.h>
#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#include <rkhv/panic.h>
#include <rkhv/vmm/vm_manager.h>

#include "vm_guest_paging.h"
#include "vmx_ept.h"

#define EPT_GET_HOSTVA(guestpa, hostva)                                                                      \
	do {                                                                                                 \
		uintptr_t hostpa;                                                                            \
		if (!vmx_ept_get_host_physical_address(vm, guestpa, &hostpa)) {                              \
			PANIC("Unable to walk EPT to get host physical address from guest physical address " \
			      "during guest identity paging setup");                                         \
		}                                                                                            \
		hostva = P2V_IDENTITY_MAP(hostpa);                                                           \
	} while (0)

uintptr_t vm_guest_paging_setup_identity(vm_t* vm) {
	if (vm->guest_physical_pages == 0) {
		PANIC("Unable to setup guest identity paging on a VM without guest physical pages allocated");
	}

	// NOTE : As explained in paging_map_physical in boot/src/paging.c : We use 2MiB pages for compatibility

	uintptr_t pml4_guestpa = (vm->guest_physical_pages - 1) * PAGE_SIZE;
	uint64_t* pml4_hostva;
	EPT_GET_HOSTVA(pml4_guestpa, pml4_hostva);

	uintptr_t pdpt_guestpa = pml4_guestpa - PAGE_SIZE;
	uint64_t* pdpt_hostva;
	EPT_GET_HOSTVA(pdpt_guestpa, pdpt_hostva);

	for (size_t pdpt_index = 0; pdpt_index < PAGE_TABLE_ENTRIES; pdpt_index++) {
		uintptr_t pd_guestpa = pdpt_guestpa - ((pdpt_index + 1) * PAGE_SIZE);
		uint64_t* pd_hostva;
		EPT_GET_HOSTVA(pd_guestpa, pd_hostva);

		for (size_t pd_index = 0; pd_index < PAGE_TABLE_ENTRIES; pd_index++) {
			uintptr_t pde_physical_address_page = ((pdpt_index * PAGE_TABLE_ENTRIES) + pd_index) << LINEAR_ADDRESS_D_SHIFT;
			pd_hostva[pd_index] = PDE_PRESENT | PDE_READ_WRITE | PDE_PAGE_SIZE |
					      (pde_physical_address_page & PDE_PHYSICAL_ADDRESS_PAGE);
		}

		pdpt_hostva[pdpt_index] = PDPTE_PRESENT | PDPTE_READ_WRITE |
					  (pd_guestpa & PDPTE_PHYSICAL_ADDRESS_PD);
	}

	memset(pml4_hostva, 0, PAGE_SIZE);
	pml4_hostva[LINEAR_ADDRESS_GET_PML4(0)] = PML4E_PRESENT | PML4E_READ_WRITE |
						  (pdpt_guestpa & PML4E_PHYSICAL_ADDRESS_PDPT);
	return pml4_guestpa;
}
#undef EPT_GET_HOSTVA
