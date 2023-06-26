#ifndef COMMON_VM_MANAGER
#define COMMON_VM_MANAGER

#include <rkhv/assert.h>

#include <rkhv/vmm/vmx_vmcs.h>

#define VM_T_FLAG_VMCS_INITIALIZED     (1 << 0)
#define VM_T_FLAG_EPT_PML4_INITIALIZED (1 << 1)

typedef enum __attribute__((packed)) vm_page_type_t {
	VM_PAGE_TYPE_EPT,
	VM_PAGE_TYPE_GUEST_PHYSICAL,
} vm_page_type_t;

typedef struct __attribute__((packed)) vm_page_list_t {
	struct vm_page_list_t* next;
	uintptr_t physical_address;
	size_t pages : 56;
	vm_page_type_t page_type : 8;
} vm_page_list_t;
static_assert(sizeof(vm_page_list_t) == 3 * sizeof(uintptr_t), "vm_page_list_t isn't properly packed");

typedef struct vm_t vm_t;
typedef void (*vm_device_outb_handler_t)(vm_t*, void*, uint16_t, uint8_t);
typedef void (*vm_device_outw_handler_t)(vm_t*, void*, uint16_t, uint16_t);
typedef void (*vm_device_outd_handler_t)(vm_t*, void*, uint16_t, uint32_t);
typedef uint8_t (*vm_device_inb_handler_t)(vm_t*, void*, uint16_t);
typedef uint16_t (*vm_device_inw_handler_t)(vm_t*, void*, uint16_t);
typedef uint32_t (*vm_device_ind_handler_t)(vm_t*, void*, uint16_t);

#define VM_DEVICE_T_MAX_PORTS 16
typedef struct vm_device_t {
	struct vm_device_t* next;
	uint16_t ports[VM_DEVICE_T_MAX_PORTS];
	size_t ports_len;

	void* device_data;

	vm_device_outb_handler_t outb_handler;
	vm_device_outw_handler_t outw_handler;
	vm_device_outd_handler_t outd_handler;
	vm_device_inb_handler_t inb_handler;
	vm_device_inw_handler_t inw_handler;
	vm_device_ind_handler_t ind_handler;
} vm_device_t;

/* TODO : Find a clean way to make this structure easily adaptable to
 *        other virtualization extensions (such as AMD SVM)
 */
typedef struct vm_t {
	struct vm_t* next;
	const char* name;
	uint64_t flags;

	uintptr_t vmcs_region;
	vmx_initial_vmcs_config_t vmcs_config;

	size_t guest_physical_pages;
	/* This is the amount of pages at the end of guest physical memory
	 * that was used by the initial setup of the guest (such as its
	 * identity paging or the structures for the boot protocol of the
	 * guest operating system)
	 */
	size_t guest_physical_pages_used_by_initial_setup;
	vm_page_list_t* tracked_pages;

	vm_device_t* devices;
} vm_t;

extern vm_t* vm_manager_vm_list;
vm_t* vm_manager_create_vmx_machine(const char* name);

uintptr_t vm_manager_get_free_guest_physical_page(void);

#endif
