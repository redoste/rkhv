#ifndef COMMON_RKHV_MEMORY_MAP_H
#define COMMON_RKHV_MEMORY_MAP_H

/*    rkhv memory map
 * +-------------------+
 * |                   | 0x0000000000000000
 * |                   |
 * |                   | 0x000000ffffffffff
 * +-------------------+
 * |                   | 0x0000010000000000
 * |  Physical Memory  |                     (512 GiB)
 * |                   | 0x0000017fffffffff
 * +-------------------+
 * |                   | 0x0000018000000000
 * |                   |
 * |                   | 0x00007fffffffffff
 * +-------------------+
 * |                   | 0xffff800000000000
 * |                   |
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
 * |                   |
 * |                   | 0xffffffffffefffff
 * +-------------------+
 * |                   | 0xfffffffffff00000
 * |  rkhv stack       |                     (1 MiB)
 * |                   | 0xffffffffffffffff
 * +-------------------+
 */

#define RKHV_PHYSICAL_MEMORY_BASE 0x0000010000000000
#define RKHV_RX_BASE              0xffffff0000000000
#define RKHV_RO_BASE              0xffffff0000100000
#define RKHV_RW_BASE              0xffffff0000200000
#define RKHV_STACK_BASE           0xfffffffffff00000

#define P2V_IDENTITY_MAP(pa) ((void*)(((uintptr_t)(pa)) + RKHV_PHYSICAL_MEMORY_BASE))
#define V2P_IDENTITY_MAP(va) (((uintptr_t)(va)) - RKHV_PHYSICAL_MEMORY_BASE)

#endif
