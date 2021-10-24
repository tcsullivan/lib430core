main:
	mov #42, &0x40
	mov #44, &0x42
	mov #46, &0x44
	mov #0x40, r4
	mov r4, r5
	mov 4(r5), r6
	mov @r5+, r7
	mov @r5, r8
	mov var, r9
	mov &0x44, r10
	ret

var: .byte 13
