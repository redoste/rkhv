#include <rkhv/cr_msr.h>
#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "vmx"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vmx_pages.h>

#include "vmx_init.h"
#include "vmx_instructions.h"

/* IA32_VMX_EPT_VPID_CAP MSR : VPID and EPT capabilities : Vol 3 Appendix A.10 */
#define IA32_VMX_EPT_VPID_CAP_EXECUTE_ONLY       (1 << 0)
#define IA32_VMX_EPT_VPID_CAP_PAGE_WALK_LENGTH_4 (1 << 6)
#define IA32_VMX_EPT_VPID_CAP_UNCACHEABLE        (1 << 8)
#define IA32_VMX_EPT_VPID_CAP_WRITE_BACK         (1 << 14)
#define IA32_VMX_EPT_VPID_CAP_2MIB_PAGES         (1 << 16)
#define IA32_VMX_EPT_VPID_CAP_1GIB_PAGES         (1 << 17)

void vmx_setup(void) {
	/* NOTE : For now we will assume only CR4.VMXE is required and not set
	 *        If the currently used CPU has an other requirement, we probably don't support this configuration thus we'll just crash miserably
	 */
	cr4_write(cr4_read() | CR4_VMXE);  // CR4.VMXE : Virtual Machine Extensions Enable
	LOG("IA32_VMX_CR0_FIXED : %zxpq:%zxpq / cr0 : %zxpq", rdmsr(IA32_VMX_CR0_FIXED0), rdmsr(IA32_VMX_CR0_FIXED1), cr0_read());
	LOG("IA32_VMX_CR4_FIXED : %zxpq:%zxpq / cr4 : %zxpq", rdmsr(IA32_VMX_CR4_FIXED0), rdmsr(IA32_VMX_CR4_FIXED1), cr4_read());

	uint64_t ia32_vmx_basic = rdmsr(IA32_VMX_BASIC);
	LOG("IA32_VMX_BASIC : %zxpq", ia32_vmx_basic);
	if (((ia32_vmx_basic >> 32) & 0x1fff) > PAGE_SIZE) {
		PANIC("Invalid VMCS region size in IA32_VMX_BASIC");
	}

	uint64_t ia32_vmx_ept_vpid_cap = rdmsr(IA32_VMX_EPT_VPID_CAP);
	LOG("IA32_VMX_EPT_VPID_CAP : %zxpq", ia32_vmx_ept_vpid_cap);
	if (!(ia32_vmx_ept_vpid_cap & IA32_VMX_EPT_VPID_CAP_PAGE_WALK_LENGTH_4)) {
		PANIC("EPT Page-walk length of 4 unsupported");
	}
	if (!(ia32_vmx_ept_vpid_cap & IA32_VMX_EPT_VPID_CAP_WRITE_BACK)) {
		PANIC("EPT paging-structure of memory type write-back unsupported");
	}
	if (!(ia32_vmx_ept_vpid_cap & IA32_VMX_EPT_VPID_CAP_2MIB_PAGES)) {
		LOG("WARNING : 2MiB EPT pages unsupported");
	}
	if (!(ia32_vmx_ept_vpid_cap & IA32_VMX_EPT_VPID_CAP_1GIB_PAGES)) {
		LOG("WARNING : 1GiB EPT pages unsupported");
	}

	uintptr_t vmxon_region = vmx_get_vmxon_region();
	VMX_ASSERT(vmx_vmxon(vmxon_region));
	LOG("vmxon done with VMXON region @ %p", (void*)vmxon_region);
}
