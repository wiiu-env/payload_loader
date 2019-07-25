OUTPUT(payload_loader.elf);

ENTRY(_start);

SECTIONS {    
	. = 0x011DD000;
	.text : {		
		*(.text*);
	}
	.data : {
		*(.rodata*);		
	}
	/DISCARD/ : {
		*(*);
	}
}

ASSERT((SIZEOF(.text) + SIZEOF(.data)) < 0x3000, "please recude the size.");
