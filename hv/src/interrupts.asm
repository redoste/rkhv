bits 64
section .text

extern interrupts_handler
extern xsave_area_size

%macro isr_backup_regs 0
	; Pushed by the interrupt :
	; ss
	; rsp
	; eflags
	; cs
	; rip
	; error code (if present)
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	; NOTE : For now we assume data segment selectors keep their default values
%endmacro

%macro isr_restore_regs 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%macro isr_stub 1
isr_stub_%1:
	isr_backup_regs
	mov rdi, %1
	mov rsi, rsp

	mov rbx, rsp
	sub rsp, [rel xsave_area_size]
	and rsp, 0xffffffffffffffc0 ; The XSAVE area must be 64-byte aligned

	xor eax, eax
	not eax
	mov edx, eax
	xsave [rsp]

	mov rbp, rsp
	and rsp, 0xfffffffffffffff0 ; Clang expect 16-byte aligned stack for doing its XMM shenanigans
	call interrupts_handler
	mov rsp, rbp

	xor eax, eax
	not eax
	mov edx, eax
	xrstor [rsp]
	mov rsp, rbx

	isr_restore_regs
	iretq
%endmacro

%assign i 0
%rep 256
	isr_stub i
%assign i i+1
%endrep

section .rodata
%macro isr_stub_ptr 1
	dq isr_stub_%1
%endmacro

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
	isr_stub_ptr i
%assign i i+1
%endrep
