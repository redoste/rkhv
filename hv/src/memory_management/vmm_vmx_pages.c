#include <stdbool.h>
#include <stddef.h>

#include <rkhv/cr_msr.h>
#include <rkhv/memory_map.h>
#include <rkhv/stdint.h>

#include <rkhv/panic.h>
#include <rkhv/vmm/vmx_pages.h>

#include "memory_management.h"
#include "vmm_vmx_pages.h"

static uintptr_t vmxon_region = (uintptr_t)NULL;

static uintptr_t vmcs_regions[VMX_MAX_VMCS];
static size_t vmcs_regions_size = 0;

bool mm_page_used_by_vmx(uintptr_t page_physical_address) {
	if (page_physical_address == vmxon_region) {
		return true;
	}

	for (size_t i = 0; i < vmcs_regions_size; i++) {
		if (page_physical_address == vmcs_regions[i]) {
			return true;
		}
	}

	return false;
}

static inline uint32_t vmx_get_vmcs_revision_identifier(void) {
	return rdmsr(IA32_VMX_BASIC) & 0x7fffffff;
}

uintptr_t vmx_get_vmxon_region(void) {
	if (vmxon_region) {
		return vmxon_region;
	}

	vmxon_region = mm_get_free_page();
	uint32_t* vmxon_region_va = P2V_IDENTITY_MAP(vmxon_region);
	*vmxon_region_va = vmx_get_vmcs_revision_identifier();
	return vmxon_region;
}

uintptr_t vmx_get_new_vmcs_region(void) {
	if (vmcs_regions_size >= VMX_MAX_VMCS) {
		PANIC("Maximum VMCS regions exceeded");
	}

	uintptr_t vmcs_region = mm_get_free_page();
	uint32_t* vmcs_region_va = P2V_IDENTITY_MAP(vmcs_region);
	*vmcs_region_va = vmx_get_vmcs_revision_identifier();

	vmcs_regions[vmcs_regions_size++] = vmcs_region;
	return vmcs_region;
}