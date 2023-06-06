#include <rkhv/cr_msr.h>
#include <rkhv/memory_map.h>
#include <rkhv/rflags.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "vmx"
#include <rkhv/interrupts.h>
#include <rkhv/panic.h>
#include <rkhv/segments.h>
#include <rkhv/stdio.h>
#include <rkhv/vmm/vm_manager.h>
#include <rkhv/vmm/vmx_pages.h>
#include <rkhv/vmm/vmx_vmcs.h>

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

void vmx_create_initialized_vmcs(vm_t* vm) {
	if (vm->flags & VM_T_FLAG_VMCS_INITIALIZED) {
		PANIC("Trying to reinitialize an already initialized VMCS");
	}

	vm->vmcs_region = vmx_get_new_vmcs_region();
	VMX_ASSERT(vmx_vmclear(vm->vmcs_region));
	VMX_ASSERT(vmx_vmptrld(vm->vmcs_region));

	idtr_t host_idtr;
	interrupts_sidt(&host_idtr);

	gdtr_t host_gdtr;
	segments_sgdt(&host_gdtr);

	const vmx_initial_vmcs_config_t* config = &vm->vmcs_config;

	/* TODO : translate gdtr offset from guest virtual to host virtual
	 *        or maybe we should always read the segments access rights from
	 *        the host GDT
	 */
	uint32_t guest_ds_access_right = vmx_read_segment_access_right_from_gdt(config->guest_state.ds,
										config->guest_state.gdtr.offset);
	uint32_t guest_cs_access_right = vmx_read_segment_access_right_from_gdt(config->guest_state.cs,
										config->guest_state.gdtr.offset);

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
		{
			VMCS_CF_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS,
			VMCS_CF_SPBVMEC_ENABLE_EPT,
			IA32_VMX_PROCBASED_CTLS2,
		},

		{
			VMCS_CF_EPT_POINTER,
			(config->eptp & VMCS_CF_EPTP_PHYSICAL_ADDRESS_PML4) |
				VMCS_CF_EPTP_PAGE_WALK_LENGTH_4 |
				VMCS_CF_EPTP_WRITE_BACK,
			0,
		},

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

		{VMCS_HOST_GDTR_BASE, (uintptr_t)host_gdtr.offset, 0},
		{VMCS_HOST_IDTR_BASE, (uintptr_t)host_idtr.offset, 0},

		{VMCS_HOST_RSP, RKHV_STACK_TOP, 0},
		{VMCS_HOST_RIP, (uintptr_t)&vmx_vmexit, 0},

		{VMCS_HOST_IA32_SYSENTER_ESP, 0, 0},
		{VMCS_HOST_IA32_SYSENTER_EIP, 0, 0},

		/* === Guest State === */
		{VMCS_GUEST_ES, config->guest_state.ds, 0},
		{VMCS_GUEST_CS, config->guest_state.cs, 0},
		{VMCS_GUEST_SS, config->guest_state.ds, 0},
		{VMCS_GUEST_DS, config->guest_state.ds, 0},
		{VMCS_GUEST_FS, config->guest_state.ds, 0},
		{VMCS_GUEST_GS, config->guest_state.ds, 0},
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

		{VMCS_GUEST_ES_ACCESS_RIGHTS, guest_ds_access_right, 0},
		{VMCS_GUEST_CS_ACCESS_RIGHTS, guest_cs_access_right, 0},
		{VMCS_GUEST_SS_ACCESS_RIGHTS, guest_ds_access_right, 0},
		{VMCS_GUEST_DS_ACCESS_RIGHTS, guest_ds_access_right, 0},
		{VMCS_GUEST_FS_ACCESS_RIGHTS, guest_ds_access_right, 0},
		{VMCS_GUEST_GS_ACCESS_RIGHTS, guest_ds_access_right, 0},
		{VMCS_GUEST_LDTR_ACCESS_RIGHTS, VMCS_UNUSABLE_SEGMENT, 0},
		{
			VMCS_GUEST_TR_ACCESS_RIGHTS,
			GDT_SEGMENT_DESCRIPTOR_ACCESS_PRESENT |
				GDT_SEGMENT_DESCRIPTOR_ACCESS_SYSTEM |
				GDT_SEGMENT_DESCRIPTOR_SYSTEM_TYPE_TSS_BUSY,
			0,
		},

		{VMCS_GUEST_CR0, config->guest_state.cr0, 0},
		{VMCS_GUEST_CR3, config->guest_state.cr3, 0},
		{VMCS_GUEST_CR4, config->guest_state.cr4, 0},

		{VMCS_GUEST_GDTR_BASE, (uintptr_t)config->guest_state.gdtr.offset, 0},
		{VMCS_GUEST_GDTR_LIMIT, (uintptr_t)config->guest_state.gdtr.size, 0},
		{VMCS_GUEST_IDTR_BASE, (uintptr_t)config->guest_state.idtr.offset, 0},
		{VMCS_GUEST_IDTR_LIMIT, (uintptr_t)config->guest_state.idtr.size, 0},

		{VMCS_GUEST_RSP, config->guest_state.rsp, 0},
		{VMCS_GUEST_RIP, config->guest_state.rip, 0},
		{VMCS_GUEST_RFLAGS, RFLAGS_RESERVED, 0},

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

	vm->flags |= VM_T_FLAG_VMCS_INITIALIZED;
}
