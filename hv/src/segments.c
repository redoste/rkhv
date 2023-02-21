#include <rkhv/stdint.h>

#define LOG_CATEGORY "segments"
#include "stdio.h"

#include "segments.h"

/* NOTE : Even if we will not edit the GDT we can't `const` it, otherwise it will be mapped in an RO page and the CPU
 *        won't be able to set the `Access` flag in the descriptors leading to a Triple Fault
 */
static gdt_segment_descriptor_t GDT[] = {
	{0},  // Null Segment
	{
		0xffff,
		0,
		0,
		GDT_SEGMENT_DESCRIPTOR_ACCESS_RW | GDT_SEGMENT_DESCRIPTOR_ACCESS_NOT_SYSTEM |
			GDT_SEGMENT_DESCRIPTOR_ACCESS_DPL(0) | GDT_SEGMENT_DESCRIPTOR_ACCESS_PRESENT,
		GDT_SEGMENT_DESCRIPTOR_FLAGS_GRANULARITY_PAGES | GDT_SEGMENT_DESCRIPTOR_FLAGS_SIZE_32BITS |
			GDT_SEGMENT_DESCRIPTOR_FLAGS_LIMIT_HIGH(0xf),
		0,
	},  // Data Segment
	{
		0xffff,
		0,
		0,
		GDT_SEGMENT_DESCRIPTOR_ACCESS_RW | GDT_SEGMENT_DESCRIPTOR_ACCESS_EXEC |
			GDT_SEGMENT_DESCRIPTOR_ACCESS_NOT_SYSTEM | GDT_SEGMENT_DESCRIPTOR_ACCESS_DPL(0) |
			GDT_SEGMENT_DESCRIPTOR_ACCESS_PRESENT,
		GDT_SEGMENT_DESCRIPTOR_FLAGS_GRANULARITY_PAGES | GDT_SEGMENT_DESCRIPTOR_FLAGS_LONG_MODE_CODE |
			GDT_SEGMENT_DESCRIPTOR_FLAGS_LIMIT_HIGH(0xf),
		0,
	},  // Code Segment
};

static const gdtr_t GDTR = {sizeof(GDT) - 1, GDT};

void segments_setup(void) {
	asm("lgdt %0" : : "m"(GDTR));
	LOG("lgdt done with GDT @ %p, switching segments", (void*)GDT);
	segments_set_new_segs(RKHV_CS, RKHV_DS);
}
