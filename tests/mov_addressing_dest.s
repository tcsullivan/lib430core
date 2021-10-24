main:
	mov #64, r4
	mov r4, r5
	mov #42, r4
	mov r4, @r5
	mov r4, 4(r5)
	mov #50, mem
	mov &mem, &0x50
	ret

mem: .byte 5
