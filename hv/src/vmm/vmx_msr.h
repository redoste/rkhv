#ifndef VMX_MSR
#define VMX_MSR

#include <rkhv/cr_msr.h>
#include <rkhv/stdint.h>

#include <rkhv/vmm/vmx_msr.h>

/* TODO : Maybe we should refactor this using a simple array instead of X-macros :
 *        it looks reasonable and easily maintainable for now but the unrolled code
 *        in vmx_msr_vm{exit,resume} might grow significantly
 */

/* VMCS : backed up and restored by VMX itself on VM-exit and VM-entry
 * RWBK : backed up and restored by vmx_msr_vmexit and vmx_msr_vmresume
 * RO   : read-only MSRs
 */

#define VMX_MSR_LIST                                           \
	X_VMCS(IA32_EFER, ia32_efer, VMCS_GUEST_IA32_EFER)     \
	X_VMCS(IA32_FS_BASE, ia32_fs_base, VMCS_GUEST_FS_BASE) \
	X_VMCS(IA32_GS_BASE, ia32_gs_base, VMCS_GUEST_GS_BASE) \
	X_RWBK(IA32_MISC_ENABLE, ia32_misc_enable)             \
	X_RWBK(IA32_BIOS_SIGN_ID, ia32_bios_sign_id)           \
	X_RWBK(IA32_XSS, ia32_xss)                             \
	X_RWBK(IA32_KERNEL_GS_BASE, ia32_kernel_gs_base)       \
	X_RO(IA32_ARCH_CAPABILITIES)

#define X_VMCS(addr, name, vmcs) uint64_t name;
#define X_RWBK(addr, name)       uint64_t name;
#define X_RO(addr)
typedef struct vmx_msr_state_t {
	VMX_MSR_LIST
} vmx_msr_state_t;
#undef X_VMCS
#undef X_RWBK
#undef X_RO

typedef struct vmx_vmexit_state_t vmx_vmexit_state_t;

void vmx_msr_vmlaunch(void);
void vmx_msr_vmexit(vmx_vmexit_state_t* vm_state);
void vmx_msr_vmresume(vmx_vmexit_state_t* vm_state);

#endif
