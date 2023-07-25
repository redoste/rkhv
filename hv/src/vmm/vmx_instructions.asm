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

extern xsave_area_size

global vmx_vmlaunch
vmx_vmlaunch:
	; We backup the callee-saved registers in case the vmlaunch fails
	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	mov rbx, rdi

	mov rcx, [rel xsave_area_size]
	mov rbp, rsp
	sub rsp, rcx
	and rsp, 0xffffffffffffffc0 ; The XSAVE area must be 64-byte aligned

	xor eax, eax
	mov rdi, rsp
	rep stosb

	; We xrstor an empty XSAVE area to reset x87 and the SIMD extensions to their initial state
	; See Intel Manual Volume 1 : Chapter 13.8 : Operation of XRSTOR
	; > If XSTATE_BV[i] = 0, the state component is set to its initial configuration.
	; Here XSTATE_BV = 0 since we zeroed all the XSAVE area
	not eax
	mov edx, eax
	xrstor [rsp]
	mov rsp, rbp

	; We received a vmx_initial_gpr_state_t as the first argument
	mov r15, [rbx +  0*8]
	mov r14, [rbx +  1*8]
	mov r13, [rbx +  2*8]
	mov r12, [rbx +  3*8]
	mov r11, [rbx +  4*8]
	mov r10, [rbx +  5*8]
	mov r9,  [rbx +  6*8]
	mov r8,  [rbx +  7*8]
	mov rdi, [rbx +  8*8]
	mov rsi, [rbx +  9*8]
	mov rbp, [rbx + 10*8]
	mov rdx, [rbx + 11*8]
	mov rcx, [rbx + 12*8]
	mov rax, [rbx + 14*8]
	mov rbx, [rbx + 13*8]

	vmlaunch

	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx
	jmp vmx_error_check
