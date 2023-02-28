#include <rkhv/cr_msr.h>
#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "vmx"
#include <rkhv/panic.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vmx_pages.h>

#include "vmx_init.h"
#include "vmx_instructions.h"

void vmx_setup(void) {
	/* NOTE : For now we will assume only CR4.VMXE is required and not set
	 *        If the currently used CPU has an other requirement, we probably don't support this configuration thus we'll just crash miserably
	 */
	cr4_write(cr4_read() | (1 << 13));  // CR4.VMXE : Virtual Machine Extensions Enable
	LOG("IA32_VMX_CR0_FIXED : %zxpq:%zxpq / cr0 : %zxpq", rdmsr(IA32_VMX_CR0_FIXED0), rdmsr(IA32_VMX_CR0_FIXED1), cr0_read());
	LOG("IA32_VMX_CR4_FIXED : %zxpq:%zxpq / cr4 : %zxpq", rdmsr(IA32_VMX_CR4_FIXED0), rdmsr(IA32_VMX_CR4_FIXED1), cr4_read());

	uint64_t ia32_vmx_basic = rdmsr(IA32_VMX_BASIC);
	LOG("IA32_VMX_BASIC : %zxpq", ia32_vmx_basic);
	if (((ia32_vmx_basic >> 32) & 0x1fff) > PAGE_SIZE) {
		PANIC("Invalid VMCS region size in IA32_VMX_BASIC");
	}

	uintptr_t vmxon_region = vmx_get_vmxon_region();
	VMX_ASSERT(vmx_vmxon(vmxon_region));
	LOG("vmxon done with VMXON region @ %p", (void*)vmxon_region);
}
