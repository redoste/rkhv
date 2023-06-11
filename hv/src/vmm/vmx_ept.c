#include <stdbool.h>

#include <rkhv/memory_map.h>
#include <rkhv/stdint.h>

#include <rkhv/panic.h>
#include <rkhv/vmm/vm_manager.h>
#include <rkhv/vmm/vmx_pages.h>

#include "vm_manager.h"
#include "vmx_ept.h"

void vmx_ept_map_page(vm_t* vm, uintptr_t guest_physical_page, uintptr_t host_physical_page) {
	const char* already_mapped_panic_str = "Remapping an already mapped guest physical page in EPT";
	size_t ept_pml4_index = GUEST_PHYSICAL_ADDRESS_GET_PML4(guest_physical_page);
	size_t ept_pdpt_index = GUEST_PHYSICAL_ADDRESS_GET_DIRECTORY_PTR(guest_physical_page);
	size_t ept_pd_index = GUEST_PHYSICAL_ADDRESS_GET_DIRECTORY(guest_physical_page);
	size_t ept_pt_index = GUEST_PHYSICAL_ADDRESS_GET_TABLE(guest_physical_page);

	uint64_t* ept_pml4;
	if (vm->flags & VM_T_FLAG_EPT_PML4_INITIALIZED) {
		ept_pml4 = P2V_IDENTITY_MAP(vm->vmcs_config.eptp);
	} else {
		ept_pml4 = vmx_get_free_ept_page();
		vm->vmcs_config.eptp = V2P_IDENTITY_MAP(ept_pml4);
		vm->flags |= VM_T_FLAG_EPT_PML4_INITIALIZED;
		vm_manager_track_ept_page(vm, ept_pml4);
	}

	uint64_t* ept_pdpt;
	if (ept_pml4[ept_pml4_index] & EPT_PML4E_READ) {
		ept_pdpt = P2V_IDENTITY_MAP(ept_pml4[ept_pml4_index] & EPT_PML4E_PHYSICAL_ADDRESS_PDPT);
	} else {
		ept_pdpt = vmx_get_free_ept_page();
		vm_manager_track_ept_page(vm, ept_pdpt);
		ept_pml4[ept_pml4_index] = EPT_PML4E_READ | EPT_PML4E_WRITE | EPT_PML4E_EXECUTE |
					   (V2P_IDENTITY_MAP(ept_pdpt) & EPT_PML4E_PHYSICAL_ADDRESS_PDPT);
	}

	uint64_t* ept_pd;
	if (ept_pdpt[ept_pdpt_index] & EPT_PDPTE_READ) {
		if (ept_pdpt[ept_pdpt_index] & EPT_PDPTE_PAGE_SIZE) {
			PANIC(already_mapped_panic_str);
		}
		ept_pd = P2V_IDENTITY_MAP(ept_pdpt[ept_pdpt_index] & EPT_PDPTE_PHYSICAL_ADDRESS_PD);
	} else {
		ept_pd = vmx_get_free_ept_page();
		vm_manager_track_ept_page(vm, ept_pd);
		ept_pdpt[ept_pdpt_index] = EPT_PDPTE_READ | EPT_PDPTE_WRITE | EPT_PDPTE_EXECUTE |
					   (V2P_IDENTITY_MAP(ept_pd) & EPT_PDPTE_PHYSICAL_ADDRESS_PD);
	}

	uint64_t* ept_pt;
	if (ept_pd[ept_pd_index] & EPT_PDE_READ) {
		if (ept_pd[ept_pd_index] & EPT_PDE_PAGE_SIZE) {
			PANIC(already_mapped_panic_str);
		}
		ept_pt = P2V_IDENTITY_MAP(ept_pd[ept_pd_index] & EPT_PDE_PHYSICAL_ADDRESS_PT);
	} else {
		ept_pt = vmx_get_free_ept_page();
		vm_manager_track_ept_page(vm, ept_pt);
		ept_pd[ept_pd_index] = EPT_PDE_READ | EPT_PDE_WRITE | EPT_PDE_EXECUTE |
				       (V2P_IDENTITY_MAP(ept_pt) & EPT_PDE_PHYSICAL_ADDRESS_PT);
	}

	if (ept_pt[ept_pt_index] & EPT_PTE_READ) {
		PANIC(already_mapped_panic_str);
	}
	ept_pt[ept_pt_index] = EPT_PTE_READ | EPT_PTE_WRITE | EPT_PTE_EXECUTE |
			       (host_physical_page & EPT_PTE_PHYSICAL_ADDRESS_PAGE);
}

bool vmx_ept_get_host_physical_address(vm_t* vm, uintptr_t guest_physical_address, uintptr_t* host_physical_address) {
	size_t guestpa_pml4 = GUEST_PHYSICAL_ADDRESS_GET_PML4(guest_physical_address);
	size_t guestpa_pdpt = GUEST_PHYSICAL_ADDRESS_GET_DIRECTORY_PTR(guest_physical_address);
	size_t guestpa_pd = GUEST_PHYSICAL_ADDRESS_GET_DIRECTORY(guest_physical_address);
	size_t guestpa_pt = GUEST_PHYSICAL_ADDRESS_GET_TABLE(guest_physical_address);
	size_t guestpa_offset = GUEST_PHYSICAL_ADDRESS_GET_OFFSET(guest_physical_address);

	if (!(vm->flags & VM_T_FLAG_EPT_PML4_INITIALIZED)) {
		return false;
	}

	uint64_t* ept_pml4 = P2V_IDENTITY_MAP(vm->vmcs_config.eptp);
	if (!(ept_pml4[guestpa_pml4] & EPT_PML4E_READ)) {
		return false;
	}

	uint64_t* ept_pdpt = P2V_IDENTITY_MAP(ept_pml4[guestpa_pml4] & EPT_PML4E_PHYSICAL_ADDRESS_PDPT);
	if (!(ept_pdpt[guestpa_pdpt] & EPT_PDPTE_READ)) {
		return false;
	}
	if (ept_pdpt[guestpa_pdpt] & EPT_PDPTE_PAGE_SIZE) {
		*host_physical_address = (ept_pdpt[guestpa_pdpt] & EPT_PDPTE_PHYSICAL_ADDRESS_PAGE) |
					 (guest_physical_address & (GUEST_PHYSICAL_ADDRESS_D_MASK |
								    GUEST_PHYSICAL_ADDRESS_TABLE_MASK |
								    GUEST_PHYSICAL_ADDRESS_OFFSET_MASK));
		return true;
	}

	uint64_t* ept_pd = P2V_IDENTITY_MAP(ept_pdpt[guestpa_pdpt] & EPT_PDPTE_PHYSICAL_ADDRESS_PD);
	if (!(ept_pd[guestpa_pd] & EPT_PDE_READ)) {
		return false;
	}
	if (ept_pd[guestpa_pd] & EPT_PDE_PAGE_SIZE) {
		*host_physical_address = (ept_pd[guestpa_pd] & EPT_PDE_PHYSICAL_ADDRESS_PAGE) |
					 (guest_physical_address & (GUEST_PHYSICAL_ADDRESS_TABLE_MASK |
								    GUEST_PHYSICAL_ADDRESS_OFFSET_MASK));
		return true;
	}

	uint64_t* ept_pt = P2V_IDENTITY_MAP(ept_pd[guestpa_pd] & EPT_PDE_PHYSICAL_ADDRESS_PT);
	if (!(ept_pt[guestpa_pt] & EPT_PTE_READ)) {
		return false;
	}

	*host_physical_address = (ept_pt[guestpa_pt] & EPT_PTE_PHYSICAL_ADDRESS_PAGE) | guestpa_offset;
	return true;
}

/* NOTE : We use 1GiB EPT pages for identity mapping. As explained in paging_map_physical in boot/src/paging.c,
 *        1GiB pages aren't supported on older hardware, thus it's very likely that 1GiB EPT pages are in a
 *        similar situation.
 *        Since EPT identity mapping will probably only be used for debugging purposes, support will be
 *        handled by the higher level hypervisor (VMX features are emulated when using nested VMs).
 */
void vmx_ept_create_identity_mapping(vm_t* vm) {
	if (vm->flags & VM_T_FLAG_EPT_PML4_INITIALIZED) {
		PANIC("Setting up a new EPT with id mapping on a VM with EPT PML4 already initialized");
	}
	uint64_t* ept_pdpt = vmx_get_free_ept_page();
	vm_manager_track_ept_page(vm, ept_pdpt);
	for (size_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
		ept_pdpt[i] = EPT_PDPTE_READ | EPT_PDPTE_WRITE | EPT_PDPTE_EXECUTE | EPT_PDPTE_PAGE_SIZE |
			      ((i << GUEST_PHYSICAL_ADDRESS_DP_SHIFT) & EPT_PDPTE_PHYSICAL_ADDRESS_PAGE);
	}

	uint64_t* ept_pml4 = vmx_get_free_ept_page();
	vm_manager_track_ept_page(vm, ept_pml4);
	ept_pml4[0] = EPT_PML4E_READ | EPT_PML4E_WRITE | EPT_PML4E_EXECUTE |
		      (V2P_IDENTITY_MAP(ept_pdpt) & EPT_PML4E_PHYSICAL_ADDRESS_PDPT);

	vm->vmcs_config.eptp = V2P_IDENTITY_MAP(ept_pml4);
	vm->flags |= VM_T_FLAG_EPT_PML4_INITIALIZED;
}
