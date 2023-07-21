#include <stdbool.h>

#include <rkhv/cr_msr.h>
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
	if (vm->guest_physical_pages_used_by_initial_setup != 0) {
		PANIC("Guest identity paging should be setup before any other steps allocating memory");
	}

	// NOTE : As explained in paging_map_physical in boot/src/paging.c : We use 2MiB pages for compatibility

	uintptr_t pml4_guestpa = (vm->guest_physical_pages - 1) * PAGE_SIZE;
	vm->guest_physical_pages_used_by_initial_setup++;
	uint64_t* pml4_hostva;
	EPT_GET_HOSTVA(pml4_guestpa, pml4_hostva);

	uintptr_t pdpt_guestpa = pml4_guestpa - PAGE_SIZE;
	vm->guest_physical_pages_used_by_initial_setup++;
	uint64_t* pdpt_hostva;
	EPT_GET_HOSTVA(pdpt_guestpa, pdpt_hostva);

	for (size_t pdpt_index = 0; pdpt_index < PAGE_TABLE_ENTRIES; pdpt_index++) {
		uintptr_t pd_guestpa = pdpt_guestpa - ((pdpt_index + 1) * PAGE_SIZE);
		vm->guest_physical_pages_used_by_initial_setup++;
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

#define EPT_GET_HOSTVA(guestpa, hostva)                                         \
	do {                                                                    \
		uintptr_t hostpa;                                               \
		if (!vmx_ept_get_host_physical_address(vm, guestpa, &hostpa)) { \
			return false;                                           \
		}                                                               \
		hostva = P2V_IDENTITY_MAP(hostpa);                              \
	} while (0)
bool vm_guest_paging_get_guest_physical_address(vm_t* vm, uintptr_t guest_cr3, uintptr_t guest_linear_addr, uintptr_t* guest_physical_address) {
	uintptr_t guestla_pml4 = LINEAR_ADDRESS_GET_PML4(guest_linear_addr);
	uintptr_t guestla_pdpt = LINEAR_ADDRESS_GET_DIRECTORY_PTR(guest_linear_addr);
	uintptr_t guestla_pd = LINEAR_ADDRESS_GET_DIRECTORY(guest_linear_addr);
	uintptr_t guestla_pt = LINEAR_ADDRESS_GET_TABLE(guest_linear_addr);
	uintptr_t guestla_offset = LINEAR_ADDRESS_GET_OFFSET(guest_linear_addr);

	// TODO : Do we need to check for the User/Supervisor bit or is the CPL checked by VMX ?

	uintptr_t pml4_guestpa = guest_cr3 & PAGE_MASK;
	uint64_t* pml4_hostva;
	EPT_GET_HOSTVA(pml4_guestpa, pml4_hostva);
	if (!(pml4_hostva[guestla_pml4] & PML4E_PRESENT)) {
		return false;
	}

	uintptr_t pdpt_guestpa = pml4_hostva[guestla_pml4] & PML4E_PHYSICAL_ADDRESS_PDPT;
	uint64_t* pdpt_hostva;
	EPT_GET_HOSTVA(pdpt_guestpa, pdpt_hostva);
	if (!(pdpt_hostva[guestla_pdpt] & PDPTE_PRESENT)) {
		return false;
	}
	if (pdpt_hostva[guestla_pdpt] & PDPTE_PAGE_SIZE) {
		*guest_physical_address = (pdpt_hostva[guestla_pdpt] & PDPTE_PHYSICAL_ADDRESS_PAGE) |
					  (guest_linear_addr & (LINEAR_ADDRESS_D_MASK |
								LINEAR_ADDRESS_TABLE_MASK |
								LINEAR_ADDRESS_OFFSET_MASK));
		return true;
	}

	uintptr_t pd_guestpa = pdpt_hostva[guestla_pdpt] & PDPTE_PHYSICAL_ADDRESS_PD;
	uint64_t* pd_hostva;
	EPT_GET_HOSTVA(pd_guestpa, pd_hostva);
	if (!(pd_hostva[guestla_pd] & PDE_PRESENT)) {
		return false;
	}
	if (pd_hostva[guestla_pd] & PDE_PAGE_SIZE) {
		*guest_physical_address = (pd_hostva[guestla_pd] & PDE_PHYSICAL_ADDRESS_PAGE) |
					  (guest_linear_addr & (LINEAR_ADDRESS_TABLE_MASK |
								LINEAR_ADDRESS_OFFSET_MASK));
		return true;
	}

	uintptr_t pt_guestpa = pd_hostva[guestla_pd] & PDE_PHYSICAL_ADDRESS_PT;
	uint64_t* pt_hostva;
	EPT_GET_HOSTVA(pt_guestpa, pt_hostva);
	if (!(pt_hostva[guestla_pt] & PTE_PRESENT)) {
		return false;
	}

	*guest_physical_address = (pt_hostva[guestla_pt] & PTE_PHYSICAL_ADDRESS_PAGE) | guestla_offset;
	return true;
}
#undef EPT_GET_HOSTVA

void* vm_guest_paging_get_host_virtual_address_during_vmexit(vmx_vmexit_state_t* vm_state, uintptr_t guest_virtual_address, size_t guest_data_size) {
	if (!(vm_state->cr0 & CR0_PG) ||
	    !(vm_state->cr4 & CR4_PAE) ||
	    (vm_state->cr4 & CR4_LA57) ||
	    !(vm_state->msrs.ia32_efer & IA32_EFER_LME)) {
		PANIC("Resolving guest virtual address in an unsupported configuration (not long mode with 4-level paging)");
	}
	// TODO : Support segmentation when long mode is not enabled
	uintptr_t guest_linear_addr = guest_virtual_address;
	uintptr_t guest_physical_address;

	// TODO : Do we need to check CPL and/or IOPL ?
	if (!vm_guest_paging_get_guest_physical_address(vm_state->vm, vm_state->cr3, guest_linear_addr, &guest_physical_address)) {
		// TODO : Do we need to #PF the guest ?
		PANIC("Unable to walk guest page table to resolve guest linear address in VM-Exit handler");
	}

	uintptr_t host_physical_address;
	if (!vmx_ept_get_host_physical_address(vm_state->vm, guest_physical_address, &host_physical_address)) {
		// TODO : Do we need to #PF the guest ?
		PANIC("Unable to walk EPT to resolve guest physical address in VM-Exit handler");
	}

	void* host_virtual_address = P2V_IDENTITY_MAP(host_physical_address);
	uintptr_t hostva_start_page = (uintptr_t)host_virtual_address & PAGE_MASK;
	uintptr_t hostva_end_page = ((uintptr_t)host_virtual_address + (guest_data_size - 1)) & PAGE_MASK;
	if (hostva_start_page != hostva_end_page) {
		PANIC("Resolving guest virtual address across page boudaries in VM-Exit handler");
	}

	return host_virtual_address;
}
