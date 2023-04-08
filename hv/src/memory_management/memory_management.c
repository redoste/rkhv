#include <rkhv/chainload.h>
#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "mm"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>

#include "arena.h"
#include "memory_management.h"
#include "vmm_vmx_pages.h"

static chainload_page_t* mm_chainload_page;

static bool mm_page_used_by_chainload(uintptr_t page_physical_address) {
	page_physical_address &= PAGE_MASK;

	if (page_physical_address == mm_chainload_page->chainload_page.physical_address) {
		return true;
	}

	for (size_t sections_index = 0; sections_index < RKHV_MAX_SECTIONS; sections_index++) {
		uintptr_t section_first_page = mm_chainload_page->rkhv_sections[sections_index].physical_address;
		uintptr_t section_last_page = section_first_page + ((mm_chainload_page->rkhv_sections[sections_index].pages - 1) * PAGE_SIZE);
		if (page_physical_address >= section_first_page && page_physical_address <= section_last_page) {
			return true;
		}
	}

	for (size_t page_table_pages_index = 0; page_table_pages_index < mm_chainload_page->page_table_pages.len; page_table_pages_index++) {
		if (mm_chainload_page->page_table_pages.physical_addresses[page_table_pages_index] == page_physical_address) {
			return true;
		}
	}

	return false;
}

static bool mm_page_is_free(uintptr_t page_physical_address) {
	static bool (*const page_used_by_functions[])(uintptr_t) = {
		mm_page_used_by_chainload,
		mm_page_used_by_arena,
		mm_page_used_by_vmx,
	};
	for (size_t i = 0; i < sizeof(page_used_by_functions) / sizeof(page_used_by_functions[0]); i++) {
		if (page_used_by_functions[i](page_physical_address)) {
			return false;
		}
	}
	return true;
}

uintptr_t mm_get_free_page(void) {
	static size_t last_returned_region_index = (size_t)-1;
	static size_t last_returned_page_index = (size_t)-1;

	size_t current_region_index = (size_t)-1;
	size_t current_page_index = (size_t)-1;

	if (last_returned_region_index == (size_t)-1 || last_returned_page_index == (size_t)-1) {
		for (size_t i = RKHV_MAX_EFI_MMAP_DESCRIPTORS - 1; i >= 0; i--) {
			if (mm_chainload_page->efi_mmap_usable[i].usable && mm_chainload_page->efi_mmap_usable[i].pages != 0) {
				current_region_index = i;
				current_page_index = mm_chainload_page->efi_mmap_usable[i].pages - 1;
				break;
			}
		}
	} else {
		current_region_index = last_returned_region_index;
		current_page_index = last_returned_page_index;
	}

	if (current_region_index == (size_t)-1 || current_page_index == (size_t)-1) {
		PANIC("Unable to find usable memory region");
	}

	while (true) {
		uintptr_t current_page = mm_chainload_page->efi_mmap_usable[current_region_index].physical_address +
					 current_page_index * PAGE_SIZE;
		if (mm_page_is_free(current_page)) {
			last_returned_region_index = current_region_index;
			last_returned_page_index = current_page_index;
			return current_page;
		}

		current_page_index--;
		if (current_page_index >= mm_chainload_page->efi_mmap_usable[current_region_index].pages) {
			current_page_index = (size_t)-1;
			do {
				if (current_region_index <= 0 || current_region_index > RKHV_MAX_EFI_MMAP_DESCRIPTORS) {
					current_region_index = RKHV_MAX_EFI_MMAP_DESCRIPTORS;
				}

				for (size_t i = current_region_index - 1; i >= 0; i--) {
					if (mm_chainload_page->efi_mmap_usable[i].usable && mm_chainload_page->efi_mmap_usable[i].pages != 0) {
						current_region_index = i;
						current_page_index = mm_chainload_page->efi_mmap_usable[i].pages - 1;
						break;
					}
				}
			} while (current_page_index == (size_t)-1);
		}
	}
	// TODO : detect OOM : currently this will loop endlessly
}

void mm_setup(chainload_page_t* chainload_page) {
	mm_chainload_page = chainload_page;

	size_t usable_pages = 0;
	for (size_t i = 0; i < sizeof(chainload_page->efi_mmap_usable) / sizeof(chainload_page->efi_mmap_usable[0]); i++) {
		chainload_mmap_desc_t *iter = &mm_chainload_page->efi_mmap_usable[i];
		if (iter->pages == 0) {
			break;
		} else if (iter->usable) {
			usable_pages += iter->pages;
		}
		LOG("EFI MMAP @ %p (%zxpq pages) : %s",
		    (void*)iter->physical_address,
		    iter->pages,
		    iter->usable ? "usable" : "reserved");
	}
	LOG("Total usable memory : %zu B / %zu KiB / %zu MiB",
	    usable_pages * PAGE_SIZE,
	    (usable_pages * PAGE_SIZE) / 1024,
	    (usable_pages * PAGE_SIZE) / 1024 / 1024);
}
