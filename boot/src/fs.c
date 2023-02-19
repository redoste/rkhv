#include <efi.h>

#include "fs.h"
#include "main.h"

EFI_STATUS fs_get_volume(EFI_HANDLE image, EFI_FILE_PROTOCOL** volume) {
	EFI_GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_LOADED_IMAGE* loaded_image;
	RETURN_EFI(EFI_ST->BootServices->HandleProtocol(image, &loaded_image_guid, (void**)&loaded_image));

	EFI_GUID simple_fs_protocol_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simple_fs;
	RETURN_EFI(EFI_ST->BootServices->HandleProtocol(loaded_image->DeviceHandle, &simple_fs_protocol_guid,
							(void**)&simple_fs));

	RETURN_EFI(simple_fs->OpenVolume(simple_fs, volume));

	return EFI_SUCCESS;
}

EFI_STATUS fs_read_entier_file_to_pages(EFI_FILE_PROTOCOL* volume,
					const CHAR16* filename,
					EFI_MEMORY_TYPE memory_type,
					size_t pages,
					void** physical_address,
					size_t* read_bytes) {
	EFI_FILE_PROTOCOL* file;
	RETURN_EFI(volume->Open(volume, &file, (CHAR16*)filename, EFI_FILE_MODE_READ, 0));

	RETURN_EFI(EFI_ST->BootServices->AllocatePages(AllocateAnyPages, memory_type, pages,
						       (EFI_PHYSICAL_ADDRESS*)physical_address));

	size_t buffer_size = pages * EFI_PAGE_SIZE;
	RETURN_EFI(file->Read(file, &buffer_size, *physical_address));
	*read_bytes = buffer_size;

	RETURN_EFI(file->Close(file));
	return EFI_SUCCESS;
}
