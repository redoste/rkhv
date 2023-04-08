#ifndef COMMON_RKHV_CHAINLOAD_H
#define COMMON_RKHV_CHAINLOAD_H

#include <stdbool.h>
#include <stdint.h>

#include <rkhv/assert.h>
#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#define RKHV_MAX_SECTIONS             4
#define RKHV_MAX_PAGE_TABLE_REGIONS   16
#define RKHV_MAX_EFI_MMAP_DESCRIPTORS 235

typedef struct __attribute__((packed)) chainload_mmap_desc_t {
	uintptr_t physical_address;
	size_t pages : 63;
	/* NOTE : We have to use a size_t because for some reason on x86_64-unknown-windows
	 *        bit-field values with different types don't pack
	 */
	size_t usable : 1;
} chainload_mmap_desc_t;

typedef struct __attribute__((packed)) chainload_page_table_region_t {
	uintptr_t physical_address;
	size_t pages;
} chainload_page_table_region_t;

typedef struct __attribute__((packed)) chainload_page_t {
	struct {
		uintptr_t physical_address;
		size_t pages;
	} rkhv_sections[RKHV_MAX_SECTIONS];
	struct {
		uintptr_t physical_address;
	} chainload_page;
	uintptr_t _pad;
	chainload_page_table_region_t page_table_regions[RKHV_MAX_PAGE_TABLE_REGIONS];
	chainload_mmap_desc_t efi_mmap_usable[RKHV_MAX_EFI_MMAP_DESCRIPTORS];
} chainload_page_t;

static_assert(sizeof(chainload_page_t) == PAGE_SIZE, "Invalid chainload_page_t size");

#endif
