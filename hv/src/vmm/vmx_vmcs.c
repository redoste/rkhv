#include <rkhv/cr_msr.h>
#include <rkhv/memory_map.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "vmx"
#include <rkhv/interrupts.h>
#include <rkhv/panic.h>
#include <rkhv/segments.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vmx_pages.h>

#include "vmx_instructions.h"
#include "vmx_vmcs.h"
#include "vmx_vmexit.h"

static inline uint32_t vmx_ensure_allowed_bits(uint64_t field_encoding, uint64_t msr, uint32_t mask) {
	uint64_t msr_value = rdmsr(msr);
	uint32_t allowed_zeros = msr_value & 0xffffffff;
	uint32_t allowed_ones = msr_value >> 32;

	uint32_t result = (mask | allowed_zeros) & allowed_ones;

	LOG("VMCS CF 0x%zxtq allowed bits checked with msr 0x%zxtq = %xud:%xud : %xpd => %xpd",
	    field_encoding, msr, allowed_ones, allowed_zeros, mask, result);
	if ((mask & allowed_ones) != mask) {
		LOG("WARNING : Potentially unsupported feature flags used on VMCS CF 0x%zxtq", field_encoding);
	}

	return result;
}

static inline uint32_t vmx_read_segment_access_right_from_gdt(uint16_t segment_selector,
							      const gdt_segment_descriptor_t* gdt) {
	const gdt_segment_descriptor_t* segment_descriptor = &gdt[segment_selector >> 3];
	return ((segment_descriptor->flags & 0xf0) << 8) | segment_descriptor->access;
}

extern void vm_guest_hello_world(void);

uintptr_t vmx_create_initialized_vmcs(void) {
	uintptr_t vmcs_region = vmx_get_new_vmcs_region();

	VMX_ASSERT(vmx_vmclear(vmcs_region));
	VMX_ASSERT(vmx_vmptrld(vmcs_region));

	idtr_t idtr;
	interrupts_sidt(&idtr);

	gdtr_t gdtr;
	segments_sgdt(&gdtr);
	uint32_t ds_access_right = vmx_read_segment_access_right_from_gdt(RKHV_DS, gdtr.offset);
	uint32_t cs_access_right = vmx_read_segment_access_right_from_gdt(RKHV_CS, gdtr.offset);

	struct {
		uint64_t field_encoding;
		uint64_t value;
		uint64_t allowed_msr;
	} default_vmcs_fields[] = {
		/* === Control fields === */
		{
			VMCS_CF_PRIMARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS,
			VMCS_CF_PPBVMEC_HLT_EXITING |
				VMCS_CF_PPBVMEC_UNCONDITIONAL_IO_EXITING |
				VMCS_CF_PPBVMEC_ACTIVATE_SECONDARY_CONTROLS,
			IA32_VMX_PROCBASED_CTLS,
		},
		{VMCS_CF_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, 0, IA32_VMX_PROCBASED_CTLS2},

		{VMCS_CF_PIN_BASED_VM_EXECUTION_CONTROLS, 0, IA32_VMX_PINBASED_CTLS},

		{VMCS_CF_PRIMARY_VM_EXIT_CONTROLS, VMCS_CF_VMEX_HOST_ADDRESS_SPACE_SIZE, IA32_VMX_EXIT_CTLS},
		{VMCS_CF_VM_EXIT_MSR_STORE_COUNT, 0, 0},
		{VMCS_CF_VM_EXIT_MSR_LOAD_COUNT, 0, 0},
		{VMCS_CF_EXCEPTION_BITMAP, 0, 0},
		{VMCS_CF_PAGE_FAULT_ERROR_CODE_MASK, 0, 0},
		{VMCS_CF_PAGE_FAULT_ERROR_CODE_MATCH, 0, 0},

		{VMCS_CF_VM_ENTRY_CONTROLS, VMCS_CF_VMEN_IA_32E_MODE_GUEST, IA32_VMX_ENTRY_CTLS},
		{VMCS_CF_VM_ENTRY_MSR_LOAD_COUNT, 0, 0},
		{VMCS_CF_VM_ENTRY_INTERRUPTION_INFORMATION_FIELD, 0, 0},

		{VMCS_CF_CR3_TARGET_COUNT, 0, 0},
		{VMCS_CF_CR0_READ_SHADOW, cr0_read(), 0},
		{VMCS_CF_CR4_READ_SHADOW, cr4_read(), 0},

		/* === Host State === */
		{VMCS_HOST_ES, RKHV_DS, 0},
		{VMCS_HOST_CS, RKHV_CS, 0},
		{VMCS_HOST_SS, RKHV_DS, 0},
		{VMCS_HOST_DS, RKHV_DS, 0},
		{VMCS_HOST_FS, RKHV_DS, 0},
		{VMCS_HOST_GS, RKHV_DS, 0},
		{VMCS_HOST_TR, RKHV_DS, 0},
		{VMCS_HOST_FS_BASE, 0, 0},
		{VMCS_HOST_GS_BASE, 0, 0},
		{VMCS_HOST_TR_BASE, 0, 0},

		{VMCS_HOST_CR0, cr0_read(), 0},
		{VMCS_HOST_CR3, cr3_read(), 0},
		{VMCS_HOST_CR4, cr4_read(), 0},

		{VMCS_HOST_GDTR_BASE, (uintptr_t)gdtr.offset, 0},
		{VMCS_HOST_IDTR_BASE, (uintptr_t)idtr.offset, 0},

		{VMCS_HOST_RSP, RKHV_STACK_TOP, 0},
		{VMCS_HOST_RIP, (uintptr_t)&vmx_vmexit, 0},

		{VMCS_HOST_IA32_SYSENTER_ESP, 0, 0},
		{VMCS_HOST_IA32_SYSENTER_EIP, 0, 0},

		/* === Guest State === */
		{VMCS_GUEST_ES, RKHV_DS, 0},
		{VMCS_GUEST_CS, RKHV_CS, 0},
		{VMCS_GUEST_SS, RKHV_DS, 0},
		{VMCS_GUEST_DS, RKHV_DS, 0},
		{VMCS_GUEST_FS, RKHV_DS, 0},
		{VMCS_GUEST_GS, RKHV_DS, 0},
		{VMCS_GUEST_TR, 0, 0},
		{VMCS_GUEST_LDTR, 0, 0},

		{VMCS_GUEST_FS_BASE, 0, 0},
		{VMCS_GUEST_GS_BASE, 0, 0},
		{VMCS_GUEST_TR_BASE, 0, 0},
		{VMCS_GUEST_LDTR_BASE, 0, 0},
		{VMCS_GUEST_CS_BASE, 0, 0},
		{VMCS_GUEST_SS_BASE, 0, 0},
		{VMCS_GUEST_DS_BASE, 0, 0},
		{VMCS_GUEST_ES_BASE, 0, 0},

		{VMCS_GUEST_FS_LIMIT, 0xffffffff, 0},
		{VMCS_GUEST_GS_LIMIT, 0xffffffff, 0},
		{VMCS_GUEST_CS_LIMIT, 0xffffffff, 0},
		{VMCS_GUEST_SS_LIMIT, 0xffffffff, 0},
		{VMCS_GUEST_DS_LIMIT, 0xffffffff, 0},
		{VMCS_GUEST_ES_LIMIT, 0xffffffff, 0},
		{VMCS_GUEST_LDTR_LIMIT, 0, 0},
		{VMCS_GUEST_TR_LIMIT, 0, 0},

		{VMCS_GUEST_ES_ACCESS_RIGHTS, ds_access_right, 0},
		{VMCS_GUEST_CS_ACCESS_RIGHTS, cs_access_right, 0},
		{VMCS_GUEST_SS_ACCESS_RIGHTS, ds_access_right, 0},
		{VMCS_GUEST_DS_ACCESS_RIGHTS, ds_access_right, 0},
		{VMCS_GUEST_FS_ACCESS_RIGHTS, ds_access_right, 0},
		{VMCS_GUEST_GS_ACCESS_RIGHTS, ds_access_right, 0},
		{VMCS_GUEST_LDTR_ACCESS_RIGHTS, VMCS_UNUSABLE_SEGMENT, 0},
		{
			VMCS_GUEST_TR_ACCESS_RIGHTS,
			GDT_SEGMENT_DESCRIPTOR_ACCESS_PRESENT |
				GDT_SEGMENT_DESCRIPTOR_ACCESS_SYSTEM |
				GDT_SEGMENT_DESCRIPTOR_SYSTEM_TYPE_TSS_BUSY,
			0,
		},

		{VMCS_GUEST_CR0, cr0_read(), 0},
		{VMCS_GUEST_CR3, cr3_read(), 0},
		{VMCS_GUEST_CR4, cr4_read(), 0},

		{VMCS_GUEST_GDTR_BASE, (uintptr_t)gdtr.offset, 0},
		{VMCS_GUEST_GDTR_LIMIT, (uintptr_t)gdtr.size, 0},
		{VMCS_GUEST_IDTR_BASE, (uintptr_t)idtr.offset, 0},
		{VMCS_GUEST_IDTR_LIMIT, (uintptr_t)idtr.size, 0},

		{VMCS_GUEST_RSP, 0, 0},  // TODO : allocate a stack for the guest
		{VMCS_GUEST_RIP, (uintptr_t)&vm_guest_hello_world, 0},
		{VMCS_GUEST_RFLAGS, (1 << 1), /* Bit 1 : reserved and set */ 0},

		{VMCS_GUEST_IA32_SYSENTER_ESP, 0, 0},
		{VMCS_GUEST_IA32_SYSENTER_EIP, 0, 0},
		{VMCS_GUEST_IA32_SYSENTER_CS, 0, 0},

		{VMCS_GUEST_DR7, 0, 0},
		{VMCS_GUEST_IA32_DEBUGCTL, 0, 0},

		{VMCS_GUEST_ACTIVITY_STATE, 0 /* 0 : Active */, 0},
		{VMCS_GUEST_INTERRUPTIBILITY_STATE, 0, 0},
		{VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, 0, 0},
		{VMCS_GUEST_VMCS_LINK_POINTER, VMCS_INVALID_LINK_POINTER, 0},
	};

	for (size_t i = 0; i < sizeof(default_vmcs_fields) / sizeof(default_vmcs_fields[0]); i++) {
		uint64_t allowed_msr = default_vmcs_fields[i].allowed_msr;
		uint64_t value = default_vmcs_fields[i].value;
		uint64_t field_encoding = default_vmcs_fields[i].field_encoding;
		if (allowed_msr != 0) {
			value = vmx_ensure_allowed_bits(field_encoding, allowed_msr, value);
		}
		VMX_ASSERT(vmx_vmwrite(field_encoding, value));
	}

	return vmcs_region;
}
