#include <efi.h>
#include <stdbool.h>

#include <rkhv/cr_msr.h>
#include <rkhv/mem.h>
#include <rkhv/memory_map.h>
#include <rkhv/paging.h>

#include "main.h"
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
	return (cr0 & CR0_PG) &&        // Paging
	       (cr4 & CR4_PAE) &&       // Physical Address Extension
	       !(cr4 & CR4_LA57) &&     // 57-Bit Linear Addresses
	       (ia32_efer & (1 << 8));  // Long Mode Enable
}

static EFI_STATUS paging_allocate_page(uint64_t** pages_out, size_t pages_count, paging_page_table_pool_t* page_table_pool) {
	EFI_PHYSICAL_ADDRESS pages_physical_address;
	RETURN_EFI(EFI_ST->BootServices->AllocatePages(AllocateAnyPages,
						       EfiLoaderData,
						       pages_count,
						       &pages_physical_address));

	// NOTE : This "region collapser" will fragment really easily, let's hope the EFI is nice when allocating pages
	bool collaped_with_existing_region = false;
	for (size_t i = 0; i < page_table_pool->size; i++) {
		chainload_page_table_region_t* iter = &page_table_pool->pool[i];
		if (pages_physical_address + (EFI_PAGE_SIZE * pages_count) == iter->physical_address) {
			iter->physical_address = pages_physical_address;
			iter->pages += pages_count;
			collaped_with_existing_region = true;
			break;
		} else if (pages_physical_address == iter->physical_address + (EFI_PAGE_SIZE * iter->pages)) {
			iter->pages += pages_count;
			collaped_with_existing_region = true;
			break;
		}
	}
	if (!collaped_with_existing_region) {
		if (page_table_pool->size >= page_table_pool->capacity) {
			return EFI_BUFFER_TOO_SMALL;
		}
		size_t new_index = page_table_pool->size++;
		page_table_pool->pool[new_index].physical_address = pages_physical_address;
		page_table_pool->pool[new_index].pages = pages_count;
	}

	*pages_out = (void*)pages_physical_address;
	memset(*pages_out, 0, EFI_PAGE_SIZE * pages_count);
	return EFI_SUCCESS;
}

EFI_STATUS paging_allocate_pml4(uint64_t** pml4, paging_page_table_pool_t* page_table_pool) {
	return paging_allocate_page(pml4, 1, page_table_pool);
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
		RETURN_EFI(paging_allocate_page(&pdpt, 1, page_table_pool));
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
		RETURN_EFI(paging_allocate_page(&pd, 1, page_table_pool));
		pdpt[pdpt_index] = PDPTE_PRESENT | PDPTE_READ_WRITE | ((uintptr_t)pd & PDPTE_PHYSICAL_ADDRESS_PD);
	}

	uint64_t* pt;
	if (pd[pd_index] & PDE_PRESENT) {
		if (pd[pd_index] & PDE_PAGE_SIZE) {
			return EFI_UNSUPPORTED;
		}
		pt = (uint64_t*)(pd[pd_index] & PDE_PHYSICAL_ADDRESS_PT);
	} else {
		RETURN_EFI(paging_allocate_page(&pt, 1, page_table_pool));
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

	/* NOTE : We used to use 1GiB pages for identity mapping but they aren't supported on "older" CPUs
	 *        (see `pdpe1gb` flag in `/proc/cpuinfo` on Linux).
	 *        2MiB pages are more expensive in terms of memory usage but they should be compatible with
	 *        every processor with 4-level paging support.
	 */

	uint64_t *pdpt, *pds;
	RETURN_EFI(paging_allocate_page(&pdpt, 1, page_table_pool));
	RETURN_EFI(paging_allocate_page(&pds, PAGE_TABLE_ENTRIES, page_table_pool));
	pml4[LINEAR_ADDRESS_GET_PML4(0x0000000000000000)] = PML4E_PRESENT | PML4E_READ_WRITE | ((uintptr_t)pdpt & PML4E_PHYSICAL_ADDRESS_PDPT);
	pml4[LINEAR_ADDRESS_GET_PML4(RKHV_PHYSICAL_MEMORY_BASE)] = PML4E_PRESENT | PML4E_READ_WRITE | ((uintptr_t)pdpt & PML4E_PHYSICAL_ADDRESS_PDPT);

	for (size_t pdpt_index = 0; pdpt_index < PAGE_TABLE_ENTRIES; pdpt_index++) {
		size_t pd_base = pdpt_index * PAGE_TABLE_ENTRIES;
		pdpt[pdpt_index] = PDPTE_PRESENT | PDPTE_READ_WRITE | ((uintptr_t)&pds[pd_base] & PDPTE_PHYSICAL_ADDRESS_PD);
		for (size_t pd_index = 0; pd_index < PAGE_TABLE_ENTRIES; pd_index++) {
			pds[pd_base + pd_index] = PDE_PRESENT | PDE_READ_WRITE | PDE_PAGE_SIZE |
						  (((pd_base + pd_index) << LINEAR_ADDRESS_D_SHIFT) & PDE_PHYSICAL_ADDRESS_PAGE);
		}
	}

	return EFI_SUCCESS;
}
