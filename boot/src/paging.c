#include <efi.h>
#include <stdbool.h>

#include <rkhv/cr_msr.h>
#include <rkhv/memory_map.h>
#include <rkhv/paging.h>

#include "main.h"
#include "mem.h"
#include "paging.h"

bool paging_check_for_supported_level(void) {
	/* We only support Level-4 paging right now, so we'll just check that the UEFI didn't enable Level-5 paging
	 * Level-3 shouldn't work in 64-bit mode but we also check this to be sure
	 */

	/* Intel Manual Volume 3 : Chapter 4.5
	 * > A logical processor uses 4-level paging if CR0.PG = 1, CR4.PAE = 1, IA32_EFER.LME = 1, and CR4.LA57 = 0.
	 */

	uint64_t cr0 = cr0_read(), cr4 = cr4_read();
	uint64_t ia32_efer = rdmsr(IA32_EFER);
	return (cr0 & (1 << 31)) &&     // PG   : Paging
	       (cr4 & (1 << 5)) &&      // PAE  : Physical Address Extension
	       !(cr4 & (1 << 12)) &&    // LA57 : 57-Bit Linear Addresses
	       (ia32_efer & (1 << 8));  // LME  : Long Mode Enable
}

static EFI_STATUS paging_allocate_page(uint64_t** page, paging_page_table_pool_t* page_table_pool) {
	if (page_table_pool->size >= page_table_pool->capacity) {
		return EFI_BUFFER_TOO_SMALL;
	}

	RETURN_EFI(
		EFI_ST->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS*)page));
	page_table_pool->pool[page_table_pool->size++] = *page;
	memset(*page, 0, EFI_PAGE_SIZE);
	return EFI_SUCCESS;
}

EFI_STATUS paging_allocate_pml4(uint64_t** pml4, paging_page_table_pool_t* page_table_pool) {
	return paging_allocate_page(pml4, page_table_pool);
}

EFI_STATUS paging_map_page(uint64_t* pml4,
			   paging_page_table_pool_t* page_table_pool,
			   uintptr_t physical_address,
			   uintptr_t virtual_address,
			   paging_permission_t permission) {
	size_t pml4_index = LINEAR_ADDRESS_GET_PML4(virtual_address);
	size_t pdpt_index = LINEAR_ADDRESS_GET_DIRECTORY_PTR(virtual_address);
	size_t pd_index = LINEAR_ADDRESS_GET_DIRECTORY(virtual_address);
	size_t pt_index = LINEAR_ADDRESS_GET_TABLE(virtual_address);

	uint64_t* pdpt;
	if (pml4[pml4_index] & PML4E_PRESENT) {
		pdpt = (uint64_t*)(pml4[pml4_index] & PML4E_PHYSICAL_ADDRESS_PDPT);
	} else {
		RETURN_EFI(paging_allocate_page(&pdpt, page_table_pool));
		pml4[pml4_index] = PML4E_PRESENT | PML4E_READ_WRITE | ((uintptr_t)pdpt & PML4E_PHYSICAL_ADDRESS_PDPT);
	}

	uint64_t* pd;
	if (pdpt[pdpt_index] & PDPTE_PRESENT) {
		if (pdpt[pdpt_index] & PDPTE_PAGE_SIZE) {
			/* NOTE : We'll try to keep the bootloader paging as simple as possible thus we only support 4
			 *        KiB pages
			 */
			return EFI_UNSUPPORTED;
		}
		pd = (uint64_t*)(pdpt[pdpt_index] & PDPTE_PHYSICAL_ADDRESS_PD);
	} else {
		RETURN_EFI(paging_allocate_page(&pd, page_table_pool));
		pdpt[pdpt_index] = PDPTE_PRESENT | PDPTE_READ_WRITE | ((uintptr_t)pd & PDPTE_PHYSICAL_ADDRESS_PD);
	}

	uint64_t* pt;
	if (pd[pd_index] & PDE_PRESENT) {
		if (pd[pd_index] & PDE_PAGE_SIZE) {
			return EFI_UNSUPPORTED;
		}
		pt = (uint64_t*)(pd[pd_index] & PDE_PHYSICAL_ADDRESS_PT);
	} else {
		RETURN_EFI(paging_allocate_page(&pt, page_table_pool));
		pd[pd_index] = PDE_PRESENT | PDE_READ_WRITE | ((uintptr_t)pt & PDE_PHYSICAL_ADDRESS_PT);
	}

	if (pt[pt_index] & PTE_PRESENT) {
		return EFI_OUT_OF_RESOURCES;
	}

	const static uint64_t PERMISSION_MASK[PAGING_PERMISSION_MAX] = {
		[PAGING_RX] = 0,
		[PAGING_RO] = PTE_EXECUTE_DISABLE,
		[PAGING_RW] = PTE_EXECUTE_DISABLE | PTE_READ_WRITE,
	};
	static_assert(sizeof(PERMISSION_MASK) / sizeof(PERMISSION_MASK[0]) == 3, "Invalid PERMISSION_MASK size");

	pt[pt_index] = PTE_PRESENT | (physical_address & PTE_PHYSICAL_ADDRESS_PAGE) | PERMISSION_MASK[permission];
	return EFI_SUCCESS;
}

// TODO : find a way to not keep all physical memory RWX once we chainloaded to rkhv kernel
EFI_STATUS paging_map_physical(uint64_t* pml4, paging_page_table_pool_t* page_table_pool) {
	if (pml4[0] & PML4E_PRESENT) {
		return EFI_OUT_OF_RESOURCES;
	}

	uint64_t* pdpt;
	RETURN_EFI(paging_allocate_page(&pdpt, page_table_pool));
	pml4[LINEAR_ADDRESS_GET_PML4(0x0000000000000000)] = PML4E_PRESENT | PML4E_READ_WRITE | ((uintptr_t)pdpt & PML4E_PHYSICAL_ADDRESS_PDPT);
	pml4[LINEAR_ADDRESS_GET_PML4(RKHV_PHYSICAL_MEMORY_BASE)] = PML4E_PRESENT | PML4E_READ_WRITE | ((uintptr_t)pdpt & PML4E_PHYSICAL_ADDRESS_PDPT);

	for (size_t page_index = 0; page_index < EFI_PAGE_SIZE / sizeof(uint64_t); page_index++) {
		pdpt[page_index] = PDPTE_PRESENT | PDPTE_READ_WRITE | PDPTE_PAGE_SIZE |
				   ((page_index << LINEAR_ADDRESS_DP_SHIFT) & PDPTE_PHYSICAL_ADDRESS_PAGE);
	}

	return EFI_SUCCESS;
}
