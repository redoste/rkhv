#ifndef INTERRUPTS_H
#define INTERRUPTS_H

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

void interrupts_setup(void);

typedef struct interrupt_state_t {
	uintptr_t r15;
	uintptr_t r14;
	uintptr_t r13;
	uintptr_t r12;
	uintptr_t r11;
	uintptr_t r10;
	uintptr_t r9;
	uintptr_t r8;
	uintptr_t rdi;
	uintptr_t rsi;
	uintptr_t rbp;
	uintptr_t rdx;
	uintptr_t rcx;
	uintptr_t rbx;
	uintptr_t rax;
	union {
		struct {
			uintptr_t rip;
			uintptr_t cs;
			uintptr_t eflags;
			uintptr_t rsp;
			uintptr_t ss;
		} without_error_code;
		struct {
			uintptr_t error_code;
			uintptr_t rip;
			uintptr_t cs;
			uintptr_t eflags;
			uintptr_t rsp;
			uintptr_t ss;
		} with_error_code;
	};
} interrupt_state_t;
void interrupts_handler(uint64_t interrupt, interrupt_state_t* interrupt_state);

extern const uintptr_t isr_stub_table[];

#define INTERRUPTS_EXCEPTIONS_MAX 32

#endif
