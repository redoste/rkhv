#ifndef VM_MANAGER
#define VM_MANAGER

#include "vmx_vmcs.h"

#define VM_T_FLAG_VMCS_INITIALIZED (1 << 0)

/* TODO : Find a clean way to make this structure easily adaptable to
 *        other virtualization extensions (such as AMD SVM)
 */
typedef struct vm_t {
	struct vm_t* next;
	const char* name;
	uint64_t flags;

	uintptr_t vmcs_region;
	vmx_initial_vmcs_config_t vmcs_config;
} vm_t;
// TODO : track EPT pages
// TODO : track guest physical pages
// TODO : use vm_t in vmexit handlers

vm_t* vm_manager_create_vmx_machine(const char* name);
__attribute__((noreturn)) void vm_manager_launch(vm_t* vm);

#endif