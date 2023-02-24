bits 64
section .text

global chainload
chainload:
	cli
	mov rdi, rcx
	mov cr3, rdx

	; Jump to the new base for physical memory starting at 0x0000010000000000
	mov rax, 0x0000010000000000 + .new_physical_base
	jmp rax
.new_physical_base:
	; And disable the old paging for physical memory based at 0
	xor rax, rax
	mov [rdx], rax
	mov cr3, rdx ; Ensure the TLB cache is updated

	mov ecx, 0xc0000080 ; IA32_EFER
	rdmsr
	or eax, (1 << 11) ; IA32_EFER.NXE : Execute-disable bit enable
	wrmsr

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
