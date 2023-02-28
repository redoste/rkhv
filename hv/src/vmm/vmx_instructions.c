#include <stddef.h>

#include <rkhv/stdint.h>

#define LOG_CATEGORY "vmx"
#include <rkhv/stdio.h>

#include "vmx_instructions.h"

// Intel Manual Volume 3 : Chapter 31.4
static const char* VMX_ERRORS[] = {
	[1] = "VMCALL executed in VMX root operation",
	[2] = "VMCLEAR with invalid physical address",
	[3] = "VMCLEAR with VMXON pointer",
	[4] = "VMLAUNCH with non-clear VMCS",
	[5] = "VMRESUME with non-launched VMCS",
	[6] = "VMRESUME after VMXOFF (VMXOFF and VMXON between VMLAUNCH and VMRESUME)",
	[7] = "VM entry with invalid control field(s)",
	[8] = "VM entry with invalid host-state field(s)",
	[9] = "VMPTRLD with invalid physical address",
	[10] = "VMPTRLD with VMXON pointer",
	[11] = "VMPTRLD with incorrect VMCS revision identifier",
	[12] = "VMREAD/VMWRITE from/to unsupported VMCS component",
	[13] = "VMWRITE to read-only VMCS component",
	[14] = NULL,
	[15] = "VMXON executed in VMX root operation",
	[16] = "VM entry with invalid executive-VMCS pointer",
	[17] = "VM entry with non-launched executive VMCS",
	[18] = "VM entry with executive-VMCS pointer not VMXON pointer (when attempting to deactivate the dual-monitor treatment of SMIs and SMM)",
	[19] = "VMCALL with non-clear VMCS (when attempting to activate the dual-monitor treatment of SMIs and SMM)",
	[20] = "VMCALL with invalid VM-exit control fields",
	[21] = NULL,
	[22] = "VMCALL with incorrect MSEG revision identifier (when attempting to activate the dual-monitor treatment of SMIs and SMM)",
	[23] = "VMXOFF under dual-monitor treatment of SMIs and SMM",
	[24] = "VMCALL with invalid SMM-monitor features (when attempting to activate the dual-monitor treatment of SMIs and SMM)",
	[25] = "VM entry with invalid VM-execution control fields in executive VMCS (when attempting to return from SMM)",
	[26] = "VM entry with events blocked by MOV SS",
	[27] = NULL,
	[28] = "Invalid operand to INVEPT/INVVPID",
};

void vmx_perror(const char* message, vmx_inst_error_t error) {
	if (error == VM_SUCCEED) {
		LOG("%s : VMsucceed", message);
	} else if (error == VM_FAIL_INVALID) {
		LOG("%s : VMfailInvalid", message);
	} else {
		const char* error_message = (size_t)error < (sizeof(VMX_ERRORS) / sizeof(VMX_ERRORS[0])) ? VMX_ERRORS[error]
													 : NULL;
		if (error_message) {
			LOG("%s : VMfailValid(%u) : %s", message, error, error_message);
		} else {
			LOG("%s : VMfailValid(%u)", message, error);
		}
	}
}
