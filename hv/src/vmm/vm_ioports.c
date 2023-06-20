#include <stddef.h>

#include <rkhv/stdint.h>

#define LOG_CATEGORY "vm"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vm_manager.h>

#include "vm_ioports.h"

#define VM_OUT_DEVICE_LOOKUP(size, type)                                                                             \
	void vm_out##size(vm_t* vm, uint16_t port, type value) {                                                     \
		for (vm_device_t* device_iter = vm->devices; device_iter != NULL; device_iter = device_iter->next) { \
			for (size_t i = 0; i < device_iter->ports_len; i++) {                                        \
				if (port == device_iter->ports[i] &&                                                 \
				    device_iter->out##size##_handler != NULL) {                                      \
					device_iter->out##size##_handler(vm, device_iter->device_data, port, value); \
					return;                                                                      \
				}                                                                                    \
			}                                                                                            \
		}                                                                                                    \
                                                                                                                     \
		LOG("out" #size " \"%s\" port=%xpw value=%xp" #size, vm->name, port, value);                         \
		PANIC("unsupported I/O port");                                                                       \
	}

VM_OUT_DEVICE_LOOKUP(b, uint8_t)
VM_OUT_DEVICE_LOOKUP(w, uint16_t)
VM_OUT_DEVICE_LOOKUP(d, uint32_t)
