#ifndef VM_IOPORTS
#define VM_IOPORTS

#include <rkhv/stdint.h>

void vmx_outb(uint16_t port, uint8_t value);
void vmx_outw(uint16_t port, uint16_t value);
void vmx_outd(uint16_t port, uint32_t value);

#endif
