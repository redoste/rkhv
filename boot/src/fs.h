#ifndef FS_H
#define FS_H

#include <efi.h>

#define RKHV_EFI_PATH "rkhv"

EFI_STATUS fs_get_volume(EFI_HANDLE image, EFI_FILE_PROTOCOL** volume);
EFI_STATUS fs_read_entier_file_to_pages(EFI_FILE_PROTOCOL* volume,
					const CHAR16* filename,
					EFI_MEMORY_TYPE memory_type,
					size_t pages,
					void** physical_address,
					size_t* read_bytes);
#endif
