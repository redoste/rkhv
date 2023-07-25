bits 64
section .text

extern vmx_vmexit_handler
extern xsave_area_size

global vmx_vmexit
vmx_vmexit:
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

	mov rcx, [rel xsave_area_size]

	mov rbx, rsp
	sub rsp, rcx
	and rsp, 0xffffffffffffffc0 ; The XSAVE area must be 64-byte aligned

	; NOTE : We need to zero the XSAVE area before use since xsave doesn't properly
	;        fill with zeros the XSAVE header, which can cause xrstor to #GP
	; See Intel Manual Volume 1 : Chapter 13.7 : Operation of XSAVE
	; > The XSAVE instruction does not write any part of the XSAVE header other than the
	; > XSTATE_BV field; in particular, it does *not* write to the XCOMP_BV field.
	xor eax, eax
	mov rdi, rsp
	rep stosb

	not eax
	mov edx, eax
	xsave [rsp]

	mov rdi, rbx
	mov rsi, rsp

	mov rbp, rsp
	and rsp, 0xfffffffffffffff0 ; Clang expect 16-byte aligned stack for doing its XMM shenanigans
	call vmx_vmexit_handler
	mov rsp, rbp

	xor eax, eax
	not eax
	mov edx, eax
	xrstor [rsp]
	mov rsp, rbx

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

	vmresume
	; TODO : handle vmresume errors
	; For now we will just cry for help and halt

	mov dx, 0x3F8 ; COM1 Data
	mov rsi, .vmresume_panic_string
	mov rcx, .vmresume_panic_string_end - .vmresume_panic_string
	rep outsb

	cli
.1:
	hlt
	jmp .1

.vmresume_panic_string: db 27, "[33;1mvmx:", 27, "[0m vmresume failed, halting", 10
.vmresume_panic_string_end:
