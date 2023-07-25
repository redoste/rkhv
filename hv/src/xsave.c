#include <stddef.h>

#include <rkhv/cpuid.h>
#include <rkhv/cr_msr.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "xsave"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>
#include <rkhv/xsave.h>

#include "xsave.h"

uint64_t xsave_host_xcr0;
size_t xsave_area_size;

void xsave_setup(void) {
	/* NOTE : We enable FXSAVE (via CR4_OSFXSR) and XSAVE (via CR4_OSXSAVE) because
	 *        CR4_OSFXSR is required to use SSE* instructions
	 */
	cr4_write(cr4_read() | CR4_OSFXSR | CR4_OSXSAVE);

	uint32_t accepted_xcr0_low, accepted_xcr0_high;
	uint32_t maximum_xsave_area_size;
	cpuid(CPUID_XSAVE_LEAF, 0, &accepted_xcr0_low, NULL, &maximum_xsave_area_size, &accepted_xcr0_high);

	xsave_host_xcr0 = ((uint64_t)accepted_xcr0_high << 32) | accepted_xcr0_low;
	xsetbv(xsave_host_xcr0);

	uint32_t current_xsave_area_size;
	cpuid(CPUID_XSAVE_LEAF, 0x00, NULL, &current_xsave_area_size, NULL, NULL);

	if (maximum_xsave_area_size != current_xsave_area_size) {
		PANIC("Unexpected value for CPUID.(EAX=0DH,ECX=0).EBX after setting XCR0");
	}
	xsave_area_size = maximum_xsave_area_size;

	LOG("xsetbv done with XCR0 : %zxpq", xgetbv());
	LOG("XSAVE area is %zu bytes long", xsave_area_size);
}
