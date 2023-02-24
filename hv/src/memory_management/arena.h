#ifndef ARENA_H
#define ARENA_H

#include <stdbool.h>

#include <rkhv/stdint.h>

typedef struct arena_header_t {
	struct arena_header_t* prev;
	struct arena_header_t* next;
	struct arena_sub_header_t* subarenas;
	size_t used_size;
} arena_header_t;

typedef struct arena_sub_header_t {
	struct arena_sub_header_t* next;
	size_t used_size;
} arena_sub_header_t;

bool mm_page_used_by_arena(uintptr_t page_physical_address);
void arena_debug_print_tree(void);

#endif
