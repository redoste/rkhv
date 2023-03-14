#ifndef COMMON_INTERRUPTS_H
#define COMMON_INTERRUPTS_H

#include <stdint.h>

#include <rkhv/assert.h>

#define IDT_LEN 256

typedef struct __attribute__((packed)) idt_gate_descriptor_t {
	uint16_t offset_low;
	uint16_t segment_selector;
	uint8_t ist;
	uint8_t bitfield;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t reserved;
} idt_gate_descriptor_t;
static_assert(sizeof(idt_gate_descriptor_t) == 0x10, "Invalid idt_gate_descriptor_t size");

typedef struct __attribute__((packed)) idtr_t {
	uint16_t size;
	idt_gate_descriptor_t* offset;
} idtr_t;

#define IDT_GATE_DESCRIPTOR_BITFIELD_PRESENT   0x80
#define IDT_GATE_DESCRIPTOR_BITFIELD_DPL(x)    (((x)&3) << 5)
#define IDT_GATE_DESCRIPTOR_BITFIELD_GATE_INT  0x0E
#define IDT_GATE_DESCRIPTOR_BITFIELD_GATE_TRAP 0x0F

static inline void interrupts_sidt(idtr_t* idtr) {
	asm("sidt %0"
	    : "=m"(*idtr));
}

#endif
