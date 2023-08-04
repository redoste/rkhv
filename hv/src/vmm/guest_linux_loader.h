#ifndef GUEST_LINUX_LOADER_H
#define GUEST_LINUX_LOADER_H

#include <rkhv/stdint.h>

#include <rkhv/vmm/vm_manager.h>

void vm_guest_linux_loader(vm_t* vm, const uint8_t* image, size_t image_size, const char* cmdline);

#endif
