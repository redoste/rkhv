#ifndef CHAINLOAD_H
#define CHAINLOAD_H

#include <efi.h>

#include <rkhv/chainload.h>

EFI_STATUS chainload(chainload_page_t* chainload_page, uint64_t* pml4);

#endif
