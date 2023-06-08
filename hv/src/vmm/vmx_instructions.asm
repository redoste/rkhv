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
	vmlaunch
	jmp vmx_error_check
