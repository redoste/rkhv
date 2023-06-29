#ifndef VM_IOPORTS
#define VM_IOPORTS

#include <rkhv/stdint.h>
#include <rkhv/vmm/vm_manager.h>

void vm_outb(vm_t* vm, uint16_t port, uint8_t value);
void vm_outw(vm_t* vm, uint16_t port, uint16_t value);
void vm_outd(vm_t* vm, uint16_t port, uint32_t value);
uint8_t vm_inb(vm_t* vm, uint16_t port);
uint16_t vm_inw(vm_t* vm, uint16_t port);
uint32_t vm_ind(vm_t* vm, uint16_t port);

#endif
