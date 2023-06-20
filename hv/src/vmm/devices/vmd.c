#include <stddef.h>

#include <rkhv/arena.h>

#include "vmd.h"

static arena_t* vmd_device_arena = NULL;

arena_t* vmd_get_device_arena(void) {
	if (!vmd_device_arena) {
		vmd_device_arena = arena_create();
	}
	return vmd_device_arena;
}
