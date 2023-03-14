bits 64
section .text

extern vmx_vmexit_handler

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
	; TODO : We should probably backup XMM (and maybe others ?) registers

	mov rdi, rsp
	mov rbp, rsp
	and rsp, 0xfffffffffffffff0 ; Clang expect 16-byte aligned stack for doing its XMM shenanigans
	call vmx_vmexit_handler
	mov rsp, rbp

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
