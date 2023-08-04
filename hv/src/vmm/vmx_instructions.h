#ifndef VMX_INSTRUCTIONS_H
#define VMX_INSTRUCTIONS_H

#include <rkhv/stdint.h>

#include <rkhv/vmm/vm_manager.h>
#include <rkhv/vmm/vmx_vmcs.h>

typedef enum vmx_inst_error_t {
	VM_SUCCEED = 0,
	VM_FAIL_INVALID = -1,
} vmx_inst_error_t;

#define VMX_ASSERT(expr)                                     \
	do {                                                 \
		vmx_inst_error_t ret = (expr);               \
		if (ret != VM_SUCCEED) {                     \
			vmx_perror(#expr, ret);              \
			PANIC("VMX_ASSERT failed : " #expr); \
		}                                            \
	} while (0)

vmx_inst_error_t vmx_vmxon(uintptr_t vmxon_region_physical_address);
vmx_inst_error_t vmx_vmclear(uintptr_t vmcs_region_physical_address);
vmx_inst_error_t vmx_vmptrld(uintptr_t vmcs_region_physical_address);
vmx_inst_error_t vmx_vmptrst(uintptr_t* vmcs_region_physical_address);
vmx_inst_error_t vmx_vmread(uint64_t field_encoding, uint64_t* value);
vmx_inst_error_t vmx_vmwrite(uint64_t field_encoding, uint64_t value);
vmx_inst_error_t vmx_vmlaunch(const vmx_initial_gpr_state_t* reg_state, const vm_permanent_state_t* permanent_state);

void vmx_perror(const char* message, vmx_inst_error_t error);

#endif
