#include <stdbool.h>

#include <rkhv/stdint.h>

#define LOG_CATEGORY "interrupts"
#include <rkhv/stdio.h>

#include "interrupts.h"
#include "segments.h"

static idt_gate_descriptor_t IDT[IDT_LEN];
static const idtr_t IDTR = {sizeof(IDT) - 1, IDT};

void interrupts_setup(void) {
	for (size_t idt_index = 0; idt_index < IDT_LEN; idt_index++) {
		IDT[idt_index].offset_low = isr_stub_table[idt_index] & 0xffff;
		IDT[idt_index].offset_mid = (isr_stub_table[idt_index] >> 16) & 0xffff;
		IDT[idt_index].offset_high = (isr_stub_table[idt_index] >> 32) & 0xffffffff;
		IDT[idt_index].segment_selector = RKHV_CS;
		IDT[idt_index].ist = 0;
		IDT[idt_index].bitfield = IDT_GATE_DESCRIPTOR_BITFIELD_PRESENT |
					  IDT_GATE_DESCRIPTOR_BITFIELD_GATE_TRAP | IDT_GATE_DESCRIPTOR_BITFIELD_DPL(0);
	}
	asm("lidt %0"
	    :
	    : "m"(IDTR));
	LOG("lidt done with IDT @ %p, enabling interrupts", (void*)IDT);
	asm("sti");
}

static const struct {
	const char* name;
	bool have_error_code;
} INTERRUPTS_EXCEPTIONS_DETAILS[] = {
	{"#DE Divide Error", false},
	{"#DB Debug Exception", false},
	{"    NMI Interrupt", false},
	{"#BP Breakpoint", false},
	{"#OF Overflow", false},
	{"#BR BOUND Range Exceeded", false},
	{"#UD Invalid Opcode", false},
	{"#NM Device Not Available", false},
	{"#DF Double Fault", true},
	{"    Coprocessor Segment Overrun", false},
	{"#TS Invalid TSS", true},
	{"#NP Segment Not Present", true},
	{"#SS Stack-Segment Fault", true},
	{"#GP General Protection", true},
	{"#PF Page Fault", true},
	{"    Reserved", false},
	{"#MF x87 FPU Floating-Point Error", false},
	{"#AC Alignment Check", true},
	{"#MC Machine Check", false},
	{"#XM SIMD Floating-Point Exception", false},
	{"#VE Virtualization Exception", false},
	{"#CP Control Protection Exception", true},
};
void interrupts_handler(uint64_t interrupt, interrupt_state_t* interrupt_state) {
	if (interrupt < INTERRUPTS_EXCEPTIONS_MAX) {
		const char* exception_name = "Reserved";
		bool have_error_code = false;
		if (interrupt < sizeof(INTERRUPTS_EXCEPTIONS_DETAILS) / sizeof(INTERRUPTS_EXCEPTIONS_DETAILS[0])) {
			exception_name = INTERRUPTS_EXCEPTIONS_DETAILS[interrupt].name;
			have_error_code = INTERRUPTS_EXCEPTIONS_DETAILS[interrupt].have_error_code;
		}
		LOG("fault : %s, halting", exception_name);
		if (have_error_code) {
			LOG("ERR: %p RIP: %p", (void*)interrupt_state->with_error_code.error_code,
			    (void*)interrupt_state->with_error_code.rip);
			LOG("CS:  %p FLG: %p", (void*)interrupt_state->with_error_code.cs,
			    (void*)interrupt_state->with_error_code.eflags);
			LOG("RSP: %p SS:  %p", (void*)interrupt_state->with_error_code.rsp,
			    (void*)interrupt_state->with_error_code.ss);
		} else {
			LOG("RIP: %p", (void*)interrupt_state->without_error_code.rip);
			LOG("CS:  %p FLG: %p", (void*)interrupt_state->without_error_code.cs,
			    (void*)interrupt_state->without_error_code.eflags);
			LOG("RSP: %p SS:  %p", (void*)interrupt_state->without_error_code.rsp,
			    (void*)interrupt_state->without_error_code.ss);
		}

		asm("cli");
		while (1) {
			asm("hlt");
		}
	} else {
		LOG("unknown interrupt : int %xpb, ignored", (int)interrupt);
	}
}
