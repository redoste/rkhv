#ifndef COMMON_ARENA_H
#define COMMON_ARENA_H

#include <rkhv/stdint.h>

typedef struct arena_t arena_t;

arena_t* arena_create(void);
void arena_destroy(arena_t* arena);
void* arena_allocate(arena_t* arena, size_t size);

#endif
