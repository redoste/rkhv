#include <stddef.h>

#include <rkhv/chainload.h>
#include <rkhv/memory_map.h>
#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "mm"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>

#include "arena.h"
#include "memory_management.h"
#include "vmm_vm_manager.h"
#include "vmm_vmx_pages.h"

static chainload_page_t* mm_chainload_page;

uint8_t* mm_get_attachment(chainload_attachment_type_t attachment_type, size_t* attachment_size) {
	for (size_t attachment_index = 0; attachment_index < RKHV_MAX_ATTACHMENTS; attachment_index++) {
		chainload_attachment_t* iter = &mm_chainload_page->attachments[attachment_index];
		if (iter->attachment_type == CHAINLOAD_ATTACHMENT_END) {
			break;
		}
		if (iter->attachment_type == attachment_type) {
			if (iter->size == 0) {
				break;
			}
			*attachment_size = iter->size;
			return P2V_IDENTITY_MAP(iter->physical_address);
		}
	}

	*attachment_size = 0;
	return NULL;
}

static bool mm_page_used_by_chainload(uintptr_t page_physical_address) {
	page_physical_address &= PAGE_MASK;

	if (page_physical_address == mm_chainload_page->chainload_page.physical_address) {
		return true;
	}

	for (size_t section_index = 0; section_index < RKHV_MAX_SECTIONS; section_index++) {
		if (mm_chainload_page->rkhv_sections[section_index].pages > 0) {
			uintptr_t section_first_page = mm_chainload_page->rkhv_sections[section_index].physical_address;
			uintptr_t section_last_page = section_first_page + ((mm_chainload_page->rkhv_sections[section_index].pages - 1) * PAGE_SIZE);
			if (page_physical_address >= section_first_page && page_physical_address <= section_last_page) {
				return true;
			}
		}
	}

	for (size_t attachment_index = 0; attachment_index < RKHV_MAX_ATTACHMENTS; attachment_index++) {
		chainload_attachment_t* iter = &mm_chainload_page->attachments[attachment_index];
		if (iter->attachment_type == CHAINLOAD_ATTACHMENT_END) {
			break;
		}
		if (iter->size > 0) {
			size_t pages = (iter->size / PAGE_SIZE) + (iter->size % PAGE_SIZE == 0 ? 0 : 1);
			uintptr_t attachment_first_page = iter->physical_address;
			uintptr_t attachment_last_page = attachment_first_page + ((pages - 1) * PAGE_SIZE);
			if (page_physical_address >= attachment_first_page && page_physical_address <= attachment_last_page) {
				return true;
			}
		}
	}

	for (size_t region_index = 0; region_index < RKHV_MAX_PAGE_TABLE_REGIONS; region_index++) {
		chainload_page_table_region_t* iter = &mm_chainload_page->page_table_regions[region_index];
		if (iter->pages > 0) {
			uintptr_t region_first_page = iter->physical_address;
			uintptr_t region_last_page = region_first_page + ((iter->pages - 1) * PAGE_SIZE);
			if (page_physical_address >= region_first_page && page_physical_address <= region_last_page) {
				return true;
			}
		}
	}

	return false;
}

static bool mm_page_is_free(uintptr_t page_physical_address) {
	static bool (*const page_used_by_functions[])(uintptr_t) = {
		mm_page_used_by_chainload,
		mm_page_used_by_arena,
		mm_page_used_by_vmx,
		mm_page_used_by_vm_manager,
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

	size_t complete_loop_region_index = (size_t)-1;
	size_t complete_loop_page_index = (size_t)-1;

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

	while (current_region_index != complete_loop_region_index || current_page_index != complete_loop_page_index) {
		if (complete_loop_region_index == (size_t)-1 || complete_loop_page_index == (size_t)-1) {
			complete_loop_region_index = current_region_index;
			complete_loop_page_index = current_page_index;
		}

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

	PANIC("OOM :(");
}

void mm_setup(chainload_page_t* chainload_page) {
	mm_chainload_page = chainload_page;

	size_t usable_pages = 0;
	for (size_t i = 0; i < sizeof(chainload_page->efi_mmap_usable) / sizeof(chainload_page->efi_mmap_usable[0]); i++) {
		chainload_mmap_desc_t* iter = &mm_chainload_page->efi_mmap_usable[i];
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

	for (size_t i = 0; i < sizeof(chainload_page->page_table_regions) / sizeof(chainload_page->page_table_regions[0]); i++) {
		chainload_page_table_region_t* iter = &mm_chainload_page->page_table_regions[i];
		if (iter->pages == 0) {
			break;
		}
		LOG("Page Table Region @ %p (%zu pages)", (void*)iter->physical_address, iter->pages);
	}
}
