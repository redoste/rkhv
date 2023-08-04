#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

#include <rkhv/chainload.h>

void mm_setup(chainload_page_t* chainload_page);
uintptr_t mm_get_free_page(void);
uint8_t* mm_get_attachment(chainload_attachment_type_t attachment_type, size_t* attachment_size);

#endif
