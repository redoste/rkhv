#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <rkhv/assert.h>
#include <rkhv/stdint.h>

#define RKHV_DS 0x08
#define RKHV_CS 0x10

typedef struct __attribute__((packed)) gdt_segment_descriptor_t {
	uint16_t segment_limit_low;
	uint16_t base_address_low;
	uint8_t base_address_mid;
	uint8_t access;
	uint8_t flags;
	uint8_t base_address_high;
} gdt_segment_descriptor_t;
static_assert(sizeof(gdt_segment_descriptor_t) == 8, "Invalid gdt_segment_descriptor_t size");

typedef struct __attribute__((packed)) gdtr_t {
	uint16_t size;
	const gdt_segment_descriptor_t* offset;
} gdtr_t;

#define GDT_SEGMENT_DESCRIPTOR_ACCESS_ACCESSED   0x01
#define GDT_SEGMENT_DESCRIPTOR_ACCESS_RW         0x02
#define GDT_SEGMENT_DESCRIPTOR_ACCESS_DIRECTION  0x04
#define GDT_SEGMENT_DESCRIPTOR_ACCESS_CONFORMING 0x04
#define GDT_SEGMENT_DESCRIPTOR_ACCESS_EXEC       0x08
#define GDT_SEGMENT_DESCRIPTOR_ACCESS_SYSTEM     0x00
#define GDT_SEGMENT_DESCRIPTOR_ACCESS_NOT_SYSTEM 0x10
#define GDT_SEGMENT_DESCRIPTOR_ACCESS_DPL(x)     (((x)&3) << 5)
#define GDT_SEGMENT_DESCRIPTOR_ACCESS_PRESENT    0x80

#define GDT_SEGMENT_DESCRIPTOR_FLAGS_GRANULARITY_BYTE  0x00
#define GDT_SEGMENT_DESCRIPTOR_FLAGS_GRANULARITY_PAGES 0x80
#define GDT_SEGMENT_DESCRIPTOR_FLAGS_SIZE_16BITS       0x00
#define GDT_SEGMENT_DESCRIPTOR_FLAGS_SIZE_32BITS       0x40
#define GDT_SEGMENT_DESCRIPTOR_FLAGS_LONG_MODE_CODE    0x20
#define GDT_SEGMENT_DESCRIPTOR_FLAGS_LIMIT_HIGH(x)     ((x)&0xf)

void segments_setup(void);
void segments_set_new_segs(uint16_t cs, uint16_t ds);

#endif
