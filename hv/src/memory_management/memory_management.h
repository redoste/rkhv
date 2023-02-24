#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

#include <rkhv/chainload.h>

void mm_setup(chainload_page_t* chainload_page);
uintptr_t mm_get_free_page(void);

#endif
