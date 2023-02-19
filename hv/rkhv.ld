ENTRY(hvmain)
OUTPUT_FORMAT(elf64-x86-64)

/*    rkhv memory map
 * +-------------------+
 * |                   | 0x0000000000000000
 * |  Physical Memory  |                     (512 GiB)
 * |                   | 0x0000007fffffffff
 * +-------------------+
 * |                   | 0x0000008000000000
 * |                   |                     (255 TiB)
 * |                   | 0x0000ffffffffffff
 * +-------------------+
 * |                   | 0xffff000000000000
 * |                   |                     (255 TiB)
 * |                   | 0xfffffeffffffffff
 * +-------------------+
 * |                   |
 * |  rkhv arenas      | TODO
 * |                   |
 * +-------------------+
 * |                   | 0xffffff0000000000
 * |  rkhv .text       |                     (1 MiB)
 * |                   | 0xffffff00000fffff
 * +-------------------+
 * |                   | 0xffffff0000100000
 * |  rkhv .rodata     |                     (1 MiB)
 * |                   | 0xffffff00001fffff
 * +-------------------+
 * |                   | 0xffffff0000200000
 * |  rkhv .data .bss  |                     (1 MiB)
 * |                   | 0xffffff00002fffff
 * +-------------------+
 * |                   | 0xffffff0000300000
 * |                   |                     (1 TiB)
 * |                   | 0xffffffffffefffff
 * +-------------------+
 * |                   | 0xfffffffffff00000
 * |  rkhv stack       |                     (1 MiB)
 * |                   | 0xffffffffffffffff
 * +-------------------+
 */

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
