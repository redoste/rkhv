bits 64
section .text

global chainload
chainload:
	cli
	mov rdi, rcx
	mov cr3, rdx

	mov rsp, 0xfffffffffffffff8
	mov rbp, rsp
	mov rax, 0xffffff0000000000
	push rax

	xor rax, rax
	xor rbx, rbx
	xor rcx, rcx
	xor rdx, rdx
	xor rsi, rsi
	; First parameter : chainload_page
	; xor rdi, rdi
	xor r8, r8
	xor r9, r9
	xor r10, r10
	xor r11, r11
	xor r12, r12
	xor r13, r13
	xor r14, r14
	xor r15, r15

	ret
