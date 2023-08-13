#include <rkhv/cr_msr.h>
#include <rkhv/mem.h>
#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#include <rkhv/panic.h>
#include <rkhv/vmm/vmx_msr.h>

#include "vmx_instructions.h"
#include "vmx_msr.h"
#include "vmx_vmcs.h"
#include "vmx_vmexit.h"

static vmx_msr_state_t vmx_msr_host_backup;

void vmx_msr_init_bitmaps(uint8_t* msr_bitmaps) {
	memset(msr_bitmaps, 0xff, PAGE_SIZE);

#define X_ALLOW_MSR(addr, rw)                                                       \
	if (addr < VMCS_MSR_BITMAPS_HIGH_BASE) {                                    \
		size_t offset = VMCS_MSR_BITMAPS_LOW_OFFSET(addr);                  \
		uint8_t mask = (uint8_t) ~(1 << VMCS_MSR_BITMAPS_LOW_SHIFT(addr));  \
		msr_bitmaps[VMCS_MSR_BITMAPS_READ_LOW + offset] &= mask;            \
		if (rw) {                                                           \
			msr_bitmaps[VMCS_MSR_BITMAPS_WRITE_LOW + offset] &= mask;   \
		}                                                                   \
	} else {                                                                    \
		size_t offset = VMCS_MSR_BITMAPS_HIGH_OFFSET(addr);                 \
		uint8_t mask = (uint8_t) ~(1 << VMCS_MSR_BITMAPS_HIGH_SHIFT(addr)); \
		msr_bitmaps[VMCS_MSR_BITMAPS_READ_HIGH + offset] &= mask;           \
		if (rw) {                                                           \
			msr_bitmaps[VMCS_MSR_BITMAPS_WRITE_HIGH + offset] &= mask;  \
		}                                                                   \
	}

#define X_VMCS(addr, name, vmcs) X_ALLOW_MSR(addr, 1)
#define X_RWBK(addr, name)       X_ALLOW_MSR(addr, 1)
#define X_RO(addr)               X_ALLOW_MSR(addr, 0)
	VMX_MSR_LIST
#undef X_VMCS
#undef X_RWBK
#undef X_RO

#undef X_ALLOW_MSR
}

void vmx_msr_vmlaunch(void) {
	/* Called before vmlaunch-ing a VM to make sure vmx_msr_host_backup is up to date :
	 * current CPU MSRs -> host_backup
	 */

#define X_VMCS(addr, name, vmcs)
#define X_RWBK(addr, name) \
	vmx_msr_host_backup.name = rdmsr(addr);
#define X_RO(addr)
	VMX_MSR_LIST
#undef X_VMCS
#undef X_RWBK
#undef X_RO
}

void vmx_msr_vmexit(vmx_vmexit_state_t* vm_state) {
	/* Called on VM-exit to update the vm_state and restore vmx_msr_host_backup :
	 * VMCS guest state -> vm_state.msrs
	 * current CPU MSRs -> vm_state.msrs
	 * host_backup      -> current CPU MSRs
	 */

#define X_VMCS(addr, name, vmcs) \
	VMX_ASSERT(vmx_vmread(vmcs, &vm_state->msrs.name));
#define X_RWBK(addr, name)                 \
	vm_state->msrs.name = rdmsr(addr); \
	wrmsr(addr, vmx_msr_host_backup.name);
#define X_RO(addr)
	VMX_MSR_LIST
#undef X_VMCS
#undef X_RWBK
#undef X_RO
}

void vmx_msr_vmresume(vmx_vmexit_state_t* vm_state) {
	/* Called on VM-entry to update vmx_msr_host_backup and restore from vm_state :
	 * vm_state.msrs    -> VMCS guest state
	 * current CPU MSRs -> host_backup
	 * vm_state.msrs    -> current CPU MSRs
	 */

#define X_VMCS(addr, name, vmcs) \
	VMX_ASSERT(vmx_vmwrite(vmcs, vm_state->msrs.name));
#define X_RWBK(addr, name)                      \
	vmx_msr_host_backup.name = rdmsr(addr); \
	wrmsr(addr, vm_state->msrs.name);
#define X_RO(addr)
	VMX_MSR_LIST
#undef X_VMCS
#undef X_RWBK
#undef X_RO
}
