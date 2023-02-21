bits 64
section .text

global segments_set_new_segs
segments_set_new_segs:
	movzx rdi, di
	push rdi
	lea rax, [rel .new_cs_set]
	push rax
	retfq

.new_cs_set:
	mov ax, si
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret
