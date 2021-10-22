SECTIONS {
	. = 0x0000;
	.text : {
		* (.text);
		. = ALIGN(8);
		* (.rodata);
	}
	. = ALIGN(8);
	.data : {
		* (.data);
		. = ALIGN(8);
		* (.bss);
	}
}

