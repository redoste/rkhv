#include <stdbool.h>
#include <stddef.h>

#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "arena"
#include <rkhv/arena.h>
#include <rkhv/panic.h>
#include <rkhv/stdio.h>

#include "arena.h"
#include "memory_management.h"

static arena_header_t* arena_head = NULL;

bool mm_page_used_by_arena(uintptr_t page_physical_address) {
	page_physical_address &= PAGE_MASK;

	for (arena_header_t* arena_it = arena_head; arena_it != NULL; arena_it = arena_it->next) {
		if (page_physical_address == (uintptr_t)arena_it) {
			return true;
		}
		for (arena_sub_header_t* sub_it = arena_it->subarenas; sub_it != NULL; sub_it = sub_it->next) {
			if (page_physical_address == (uintptr_t)sub_it) {
				return true;
			}
		}
	}

	return false;
}

// TODO : support aligned allocations
void* arena_allocate(arena_t* arena, size_t size) {
	arena_header_t* arena_header = (arena_header_t*)arena;
	if (PAGE_SIZE - (arena_header->used_size + sizeof(arena_header_t)) < size) {
		PANIC("subarenas not supported right now");  // TODO
	}

	void* ret = ((uint8_t*)arena_header) + (arena_header->used_size + sizeof(arena_header_t));
	arena_header->used_size += size;
	return ret;
}

void arena_destroy(arena_t* arena) {
	arena_header_t* arena_header = (arena_header_t*)arena;
	if (arena_header->next) {
		if (arena_header->next->prev != arena_header) {
			PANIC("Corrupted arena linked list");
		}
		arena_header->next->prev = arena_header->prev;
	}

	if (arena_header->prev) {
		if (arena_header->prev->next != arena_header) {
			PANIC("Corrupted arena linked list");
		}
		arena_header->prev->next = arena_header->next;
	}

	if (arena_head == arena_header) {
		if (arena_header->prev != NULL) {
			PANIC("Corrupted arena linked list");
		}
		arena_head = arena_header->next;
	}
}

arena_t* arena_create(void) {
	arena_header_t* arena_page = (arena_header_t*)mm_get_free_page();
	arena_page->prev = NULL;
	arena_page->next = arena_head;
	arena_page->subarenas = NULL;
	arena_page->used_size = 0;

	if (arena_head != NULL) {
		if (arena_head->prev != NULL) {
			PANIC("Corrupted arena linked list");
		}
		arena_head->prev = arena_page;
	}
	arena_head = arena_page;

	return (arena_t*)arena_page;
}

void arena_debug_print_tree(void) {
	LOG("arena_head = %p", (void*)arena_head);
	for (arena_header_t* arena_it = arena_head; arena_it != NULL; arena_it = arena_it->next) {
		LOG("arena @ %p", (void*)arena_it);
		LOG(" - prev      : %p", (void*)arena_it->prev);
		LOG(" - next      : %p", (void*)arena_it->next);
		LOG(" - subarenas : %p", (void*)arena_it->subarenas);
		for (arena_sub_header_t* sub_it = arena_it->subarenas; sub_it != NULL; sub_it = sub_it->next) {
			LOG("     subarena @ %p", (void*)sub_it);
			LOG("      - next      : %p", (void*)sub_it->next);
			LOG("      - used_size : %zu", sub_it->used_size);
		}
		LOG(" - used_size : %zu", arena_it->used_size);
	}
}
