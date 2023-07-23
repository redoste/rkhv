bits 64
section .text

vmx_error_check:
	jc .vmfailinvalid
	jz .vmfailvalid
	xor eax, eax
	ret
.vmfailinvalid:
	mov eax, -1
	ret
.vmfailvalid:
	mov eax, -1
	mov rcx, 0x00004400
	; TODO : find a clean way to recover the VM-instruction error field when the current-VMCS pointer is not set
	vmread rax, rcx
	ret

global vmx_vmxon
vmx_vmxon:
	push rdi
	vmxon [rsp]
	pop rdi
	jmp vmx_error_check

global vmx_vmclear
vmx_vmclear:
	push rdi
	vmclear [rsp]
	pop rdi
	jmp vmx_error_check

global vmx_vmptrld
vmx_vmptrld:
	push rdi
	vmptrld [rsp]
	pop rdi
	jmp vmx_error_check

global vmx_vmptrst
vmx_vmptrst:
	vmptrst [rdi]
	jmp vmx_error_check

global vmx_vmread
vmx_vmread:
	vmread [rsi], rdi
	jmp vmx_error_check

global vmx_vmwrite
vmx_vmwrite:
	vmwrite rdi, rsi
	jmp vmx_error_check

global vmx_vmlaunch
vmx_vmlaunch:
	; We backup the callee-saved registers in case the vmlaunch fails
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	; We received a vmx_initial_gpr_state_t as the first argument
	mov r15, [rdi +  0*8]
	mov r14, [rdi +  1*8]
	mov r13, [rdi +  2*8]
	mov r12, [rdi +  3*8]
	mov r11, [rdi +  4*8]
	mov r10, [rdi +  5*8]
	mov r9,  [rdi +  6*8]
	mov r8,  [rdi +  7*8]
	mov rsi, [rdi +  9*8]
	mov rbp, [rdi + 10*8]
	mov rdx, [rdi + 11*8]
	mov rcx, [rdi + 12*8]
	mov rbx, [rdi + 13*8]
	mov rax, [rdi + 14*8]
	mov rdi, [rdi +  8*8]

	vmlaunch

	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	jmp vmx_error_check
