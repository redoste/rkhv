#ifndef COMMON_VM_MANAGER
#define COMMON_VM_MANAGER

#include <rkhv/assert.h>

#include <rkhv/vmm/vmx_vmcs.h>

#define VM_T_FLAG_VMCS_INITIALIZED (1 << 0)

typedef enum __attribute__((packed)) vm_page_type_t {
	VM_PAGE_TYPE_EPT,
} vm_page_type_t;

typedef struct __attribute__((packed)) vm_page_list_t {
	struct vm_page_list_t* next;
	uintptr_t physical_address;
	size_t pages : 56;
	vm_page_type_t page_type : 8;
} vm_page_list_t;
static_assert(sizeof(vm_page_list_t) == 3 * sizeof(uintptr_t), "vm_page_list_t isn't properly packed");

/* TODO : Find a clean way to make this structure easily adaptable to
 *        other virtualization extensions (such as AMD SVM)
 */
typedef struct vm_t {
	struct vm_t* next;
	const char* name;
	uint64_t flags;

	uintptr_t vmcs_region;
	vmx_initial_vmcs_config_t vmcs_config;

	vm_page_list_t* tracked_pages;
} vm_t;
// TODO : track guest physical pages
// TODO : use vm_t in vmexit handlers

extern vm_t* vm_manager_vm_list;
vm_t* vm_manager_create_vmx_machine(const char* name);

#endif
