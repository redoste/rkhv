global vm_guest_hello_world
vm_guest_hello_world:
	mov rsi, .hello_world_text
.0:
	mov al, [rsi]
	test al, al
	jz .1
	mov dx, 0x3F8 ; COM1 Data
	out dx, al
	inc rsi
	jmp .0
.1:
	hlt
	jmp .1

.hello_world_text: db "**** Start apprication ****", 10, 0
