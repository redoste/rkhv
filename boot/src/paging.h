#ifndef PAGING_H
#define PAGING_H

#include <efi.h>
#include <stdbool.h>

typedef struct paging_page_table_pool_t {
	size_t size;
	size_t capacity;
	uint64_t** pool;
} paging_page_table_pool_t;

typedef enum paging_permission_t {
	PAGING_RX,
	PAGING_RO,
	PAGING_RW,
	PAGING_PERMISSION_MAX,
} paging_permission_t;

bool paging_check_for_supported_level(void);
EFI_STATUS paging_allocate_pml4(uint64_t** pml4, paging_page_table_pool_t* page_table_pool);
EFI_STATUS paging_map_page(uint64_t* pml4,
			   paging_page_table_pool_t* page_table_pool,
			   uintptr_t physical_address,
			   uintptr_t virtual_address,
			   paging_permission_t permission);
EFI_STATUS paging_map_physical(uint64_t* pml4, paging_page_table_pool_t* page_table_pool);

#endif
