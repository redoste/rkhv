global vm_guest_hello_world
vm_guest_hello_world:
	mov rsi, .hello_world_text
	mov dx, 0x3F8 ; COM1 Data

	mov rcx, .hello_world_text_end - .hello_world_text
	rep outsb

.1:
	hlt
	jmp .1

.hello_world_text: db "**** Start apprication ****", 10
.hello_world_text_end:
