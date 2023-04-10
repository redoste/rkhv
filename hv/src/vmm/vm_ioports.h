#ifndef VM_IOPORTS
#define VM_IOPORTS

#include <rkhv/stdint.h>

void vm_outb(uint16_t port, uint8_t value);
void vm_outw(uint16_t port, uint16_t value);
void vm_outd(uint16_t port, uint32_t value);

#endif
