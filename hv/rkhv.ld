ENTRY(hvmain)
OUTPUT_FORMAT(elf64-x86-64)

MEMORY {
	RX (rx) : org = 0xffffff0000000000, len = 1M
	RO (r)  : org = 0xffffff0000100000, len = 1M
	RW (w)  : org = 0xffffff0000200000, len = 1M
}

SECTIONS {
	.text : {
		*(.text.entry)
		*(.text)
		*(.text.*)
	} > RX

	.rodata : {
		*(.rodata)
		*(.rodata.*)
	} > RO

	.data : {
		*(.data)
		*(.data.*)
	} > RW
	.bss : {
		*(.bss)
		*(.bss.*)
	} > RW

	/DISCARD/ : {
		*(.note.*)
	}
}
