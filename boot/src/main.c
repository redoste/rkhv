#include <efi.h>

#include <rkhv/chainload.h>
#define COMMON_MEM_IMPLEMENTATION
#include <rkhv/mem.h>
#include <rkhv/memory_map.h>

#include "chainload.h"
#include "fs.h"
#include "main.h"
#include "paging.h"
#include "stdio.h"
#include "string.h"

EFI_SYSTEM_TABLE* EFI_ST;

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
	EFI_ST = system_table;

	VERIFY_EFI(stdio_print_banner());

	chainload_page_t* chainload_page;
	VERIFY_EFI(EFI_ST->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData,
						       sizeof(*chainload_page) / EFI_PAGE_SIZE,
						       (EFI_PHYSICAL_ADDRESS*)&chainload_page));
	memset(chainload_page, 0, sizeof(*chainload_page));
	chainload_page->chainload_page.physical_address = (uintptr_t)chainload_page;

	EFI_FILE_PROTOCOL* volume;
	VERIFY_EFI(fs_get_volume(image_handle, &volume));

	const struct {
		const CHAR16* filename;
		EFI_MEMORY_TYPE memory_type;
		uintptr_t virtual_address;
		size_t size_in_pages;
		paging_permission_t permission;
	} sections[RKHV_MAX_SECTIONS] = {
		{u"\\" RKHV_EFI_PATH "\\rkhv.rx", EfiLoaderCode, RKHV_RX_BASE, SECTION_PAGES, PAGING_RX},
		{u"\\" RKHV_EFI_PATH "\\rkhv.ro", EfiLoaderData, RKHV_RO_BASE, SECTION_PAGES, PAGING_RO},
		{u"\\" RKHV_EFI_PATH "\\rkhv.rw", EfiLoaderData, RKHV_RW_BASE, SECTION_PAGES, PAGING_RW},
		{u"stack", EfiLoaderData, RKHV_STACK_BASE, STACK_PAGES, PAGING_RW},
	};
	uintptr_t sections_physical_addresses[RKHV_MAX_SECTIONS];
	for (size_t section_index = 0; section_index < sizeof(sections) / sizeof(sections[0]); section_index++) {
		void* physical_address;
		size_t initialized_size;
		size_t total_size_in_pages = sections[section_index].size_in_pages,
		       total_size = total_size_in_pages * EFI_PAGE_SIZE;

		if (sections[section_index].filename[0] == '\\') {
			VERIFY_EFI(stdio_puts(u"Loading "));
			VERIFY_EFI(stdio_puts(sections[section_index].filename));
			VERIFY_EFI(stdio_puts(u"..."));

			VERIFY_EFI(fs_read_entier_file_to_pages(
				volume, sections[section_index].filename, sections[section_index].memory_type,
				total_size_in_pages, &physical_address, &initialized_size));
		} else {
			VERIFY_EFI(stdio_puts(u"Allocating "));
			VERIFY_EFI(stdio_puts(sections[section_index].filename));
			VERIFY_EFI(EFI_ST->BootServices->AllocatePages(
				AllocateAnyPages, sections[section_index].memory_type, total_size_in_pages,
				(EFI_PHYSICAL_ADDRESS*)&physical_address));
			initialized_size = 0;
		}

		CHAR16 buffer[] = u" : 0xxxxxxxxx / 0xxxxxxxxx B @ 0xxxxxxxxxxxxxxxxx physical\r\n";
		str_itouh(&buffer[5], initialized_size, 8);
		str_itouh(&buffer[18], total_size, 8);
		str_itouh(&buffer[33], (uintptr_t)physical_address, 16);
		VERIFY_EFI(stdio_puts(buffer));

		sections_physical_addresses[section_index] = (uintptr_t)physical_address;
		memset((uint8_t*)physical_address + initialized_size, 0, total_size - initialized_size);

		chainload_page->rkhv_sections[section_index].physical_address = (uintptr_t)physical_address;
		chainload_page->rkhv_sections[section_index].pages = total_size_in_pages;
	}

	const struct {
		const CHAR16* filename;
		chainload_attachment_type_t type;
	} attachments[RKHV_MAX_ATTACHMENTS] = {
		{u"\\" RKHV_EFI_PATH "\\linux-bzImage", CHAINLOAD_ATTACHMENT_LINUX_BZIMAGE},
		{NULL, CHAINLOAD_ATTACHMENT_END},
	};
	for (size_t attachment_index = 0; attachment_index < sizeof(attachments) / sizeof(attachments[0]); attachment_index++) {
		const CHAR16* filename = attachments[attachment_index].filename;
		chainload_attachment_type_t type = attachments[attachment_index].type;
		if (filename == NULL || type == CHAINLOAD_ATTACHMENT_END) {
			chainload_page->attachments[attachment_index].attachment_type = CHAINLOAD_ATTACHMENT_END;
			break;
		}

		VERIFY_EFI(stdio_puts(u"Loading attachment "));
		VERIFY_EFI(stdio_puts(filename));
		VERIFY_EFI(stdio_puts(u"..."));

		size_t attachment_size;
		void* physical_address;
		EFI_STATUS read_attachment_ret = fs_read_attachment(volume, filename, &physical_address, &attachment_size);
		if (read_attachment_ret == EFI_NOT_FOUND) {
			VERIFY_EFI(stdio_puts(u" : Not Found\r\n"));
			chainload_page->attachments[attachment_index].size = 0;
			chainload_page->attachments[attachment_index].attachment_type = type;
		} else {
			VERIFY_EFI(read_attachment_ret);

			CHAR16 buffer[] = u" : 0xxxxxxxxx B @ 0xxxxxxxxxxxxxxxxx physical\r\n";
			str_itouh(&buffer[5], attachment_size, 8);
			str_itouh(&buffer[20], (uintptr_t)physical_address, 16);
			VERIFY_EFI(stdio_puts(buffer));

			chainload_page->attachments[attachment_index].physical_address = (uintptr_t)physical_address;
			chainload_page->attachments[attachment_index].size = attachment_size;
			chainload_page->attachments[attachment_index].attachment_type = type;
		}
	}

	VERIFY_EFI(volume->Close(volume));

	paging_page_table_pool_t page_table_pool = {
		.size = 0,
		.capacity = PAGE_TABLE_POOL_DEFAULT_CAPACITY,
		.pool = chainload_page->page_table_regions,
	};

	uint64_t* pml4;
	VERIFY_EFI(paging_allocate_pml4(&pml4, &page_table_pool));

	for (size_t section_index = 0; section_index < sizeof(sections) / sizeof(sections[0]); section_index++) {
		CHAR16 buffer[] =
			u"Building page table mapping 0xxxxxxxxxxxxxxxxx virtual to 0xxxxxxxxxxxxxxxxx physical\r\n";
		str_itouh(&buffer[30], sections[section_index].virtual_address, 16);
		str_itouh(&buffer[60], sections_physical_addresses[section_index], 16);
		VERIFY_EFI(stdio_puts(buffer));
		for (size_t page_index = 0; page_index < sections[section_index].size_in_pages; page_index++) {
			VERIFY_EFI(paging_map_page(
				pml4, &page_table_pool,
				sections_physical_addresses[section_index] + (page_index * EFI_PAGE_SIZE),
				sections[section_index].virtual_address + (page_index * EFI_PAGE_SIZE),
				sections[section_index].permission));
		}
	}

	VERIFY_EFI(stdio_puts(u"Building page table identity mapping for physical memory\r\n"));
	VERIFY_EFI(paging_map_physical(pml4, &page_table_pool));

	if (!paging_check_for_supported_level()) {
		VERIFY_EFI(stdio_puts(u"Invalid paging setup by firmware, only Level-4 paging is supported\r\n"));
		VERIFY_EFI(EFI_UNSUPPORTED);
	}

	VERIFY_EFI(stdio_puts(u"Exiting boot services and chainloading to rkhv...\r\n"));

	size_t mmap_size, desc_size;
	EFI_MEMORY_DESCRIPTOR* mmap;
	EFI_STATUS exit_boot_services_ret;
	do {
		uintptr_t mmap_key;
		uint32_t desc_ver;

		EFI_ST->BootServices->GetMemoryMap(&mmap_size, NULL, &mmap_key, &desc_size, &desc_ver);
		mmap_size += 4 * desc_size;  // We add 4 descriptors to be safe as they can be used by the following AllocatePool
		VERIFY_EFI(EFI_ST->BootServices->AllocatePool(EfiLoaderData, mmap_size, (void**)&mmap));
		VERIFY_EFI(EFI_ST->BootServices->GetMemoryMap(&mmap_size, mmap, &mmap_key, &desc_size, &desc_ver));

		exit_boot_services_ret = EFI_ST->BootServices->ExitBootServices(image_handle, mmap_key);

		if (exit_boot_services_ret != EFI_SUCCESS) {
			VERIFY_EFI(stdio_perror(u"ExitBootServices failed ", exit_boot_services_ret));
			VERIFY_EFI(EFI_ST->BootServices->FreePool(mmap));
		}
	} while (exit_boot_services_ret != EFI_SUCCESS);

	for (size_t efi_mmap_index = 0, out_mmap_index = 0; efi_mmap_index < mmap_size / desc_size; efi_mmap_index++) {
		// We need to do this because it's likely that desc_size != sizeof(EFI_MEMORY_DESCRIPTOR)
		EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)mmap + (efi_mmap_index * desc_size));
		bool usable;
		switch (desc->Type) {
			case EfiLoaderCode:
			case EfiLoaderData:
			case EfiBootServicesCode:
			case EfiBootServicesData:
			case EfiConventionalMemory:
				usable = true;
				break;
			case EfiReservedMemoryType:
			case EfiRuntimeServicesCode:
			case EfiRuntimeServicesData:
			case EfiUnusableMemory:
			case EfiACPIReclaimMemory:
			case EfiACPIMemoryNVS:
			case EfiMemoryMappedIO:
			case EfiMemoryMappedIOPortSpace:
			case EfiPalCode:
			default:
				usable = false;
				break;
		}

		chainload_mmap_desc_t* prev_chainload_descriptor = &chainload_page->efi_mmap_usable[out_mmap_index];
		if (efi_mmap_index != 0 && prev_chainload_descriptor->usable == (int)usable &&
		    prev_chainload_descriptor->physical_address + (prev_chainload_descriptor->pages * EFI_PAGE_SIZE) ==
			    desc->PhysicalStart) {
			prev_chainload_descriptor->pages += desc->NumberOfPages;
		} else {
			if (efi_mmap_index != 0) {
				out_mmap_index++;
			}
			if (out_mmap_index >= RKHV_MAX_EFI_MMAP_DESCRIPTORS) {
				/* If we have that many descriptors, let's hope rkhv doesn't learn about them and break
				 * something
				 */
				break;
			}

			chainload_page->efi_mmap_usable[out_mmap_index].physical_address = desc->PhysicalStart;
			chainload_page->efi_mmap_usable[out_mmap_index].pages = desc->NumberOfPages;
			chainload_page->efi_mmap_usable[out_mmap_index].usable = (int)usable;
		}
	}

	return chainload(P2V_IDENTITY_MAP(chainload_page), pml4);
}
