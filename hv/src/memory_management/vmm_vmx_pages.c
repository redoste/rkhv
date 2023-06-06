#include <stdbool.h>
#include <stddef.h>

#include <rkhv/cr_msr.h>
#include <rkhv/mem.h>
#include <rkhv/memory_map.h>
#include <rkhv/stdint.h>

#include <rkhv/panic.h>
#include <rkhv/vmm/vmx_pages.h>

#include "memory_management.h"
#include "vmm_vmx_pages.h"

static uintptr_t vmxon_region = (uintptr_t)NULL;

static uintptr_t ept_pages[VMX_MAX_EPT_PAGES];
static size_t ept_pages_size = 0;

bool mm_page_used_by_vmx(uintptr_t page_physical_address) {
	if (page_physical_address == vmxon_region) {
		return true;
	}

	for (size_t i = 0; i < ept_pages_size; i++) {
		if (page_physical_address == ept_pages[i]) {
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

uintptr_t vmx_get_free_vmcs_region(void) {
	uintptr_t vmcs_region = mm_get_free_page();
	uint32_t* vmcs_region_va = P2V_IDENTITY_MAP(vmcs_region);
	*vmcs_region_va = vmx_get_vmcs_revision_identifier();

	return vmcs_region;
}

uint64_t* vmx_allocate_ept_page(void) {
	if (ept_pages_size >= VMX_MAX_EPT_PAGES) {
		PANIC("Maximum EPT pages exceeded");
	}

	uintptr_t ept_page = mm_get_free_page();
	ept_pages[ept_pages_size++] = ept_page;

	memset(P2V_IDENTITY_MAP(ept_page), 0, PAGE_SIZE);

	return P2V_IDENTITY_MAP(ept_page);
}
