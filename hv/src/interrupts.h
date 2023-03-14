#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

#include <rkhv/interrupts.h>

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
