#include <stddef.h>

#include <rkhv/cr_msr.h>
#include <rkhv/mem.h>
#include <rkhv/memory_map.h>
#include <rkhv/paging.h>
#include <rkhv/stdint.h>

#define LOG_CATEGORY "linux"
#include <rkhv/interrupts.h>
#include <rkhv/panic.h>
#include <rkhv/segments.h>
#include <rkhv/stdio.h>
#include <rkhv/string.h>
#include <rkhv/vmm/vm_manager.h>

#include "guest_linux_loader.h"
#include "guest_linux_loader_defs.h"
#include "vm_guest_paging.h"
#include "vmx_ept.h"

// The Linux (v5.6) x86 Boot Protocol is documented here : https://www.kernel.org/doc/html/v5.6/x86/boot.html

void vm_guest_linux_loader(vm_t* vm, const uint8_t* image, size_t image_size, const char* cmdline) {
	if (vm->vmcs_config.guest_state.cr3 != 0) {
		PANIC("Guest identity paging already setup before loading guest Linux kernel");
	}
	vm->vmcs_config.guest_state.cr3 = vm_guest_paging_setup_identity(vm);

	const linux_setup_header_t* image_setup_header = (linux_setup_header_t*)(image + offsetof(linux_boot_params_t, hdr));
	// The size of the setup_header is calculated using the short jump (eb XX) at the end of {the old floppy boot sector,the PE header for the EFI stub}
	size_t image_setup_header_size = (image_setup_header->jump >> 8) + offsetof(linux_setup_header_t, jump) + sizeof(image_setup_header->jump);
	if (image_setup_header->header != 0x53726448) {
		PANIC("Linux kernel magic `HdrS` not found");
	}
	if ((image_setup_header->xloadflags & 1) != 1) {  // XLF_KERNEL_64
		PANIC("The loaded Linux kernel doesn't have a 64-bit entry point");
	}

	uintptr_t zeropage_guestpa = (vm->guest_physical_pages - vm->guest_physical_pages_used_by_initial_setup - 1) * PAGE_SIZE;
	vm->guest_physical_pages_used_by_initial_setup++;
	uintptr_t zeropage_hostpa;
	if (!vmx_ept_get_host_physical_address(vm, zeropage_guestpa, &zeropage_hostpa)) {
		PANIC("Unable to walk EPT to get Linux zeropage host physical address");
	}
	linux_boot_params_t* zeropage_hostva = P2V_IDENTITY_MAP(zeropage_hostpa);

	uintptr_t cmdline_page_guestpa = (vm->guest_physical_pages - vm->guest_physical_pages_used_by_initial_setup - 1) * PAGE_SIZE;
	vm->guest_physical_pages_used_by_initial_setup++;
	uintptr_t cmdline_page_hostpa;
	if (!vmx_ept_get_host_physical_address(vm, cmdline_page_guestpa, &cmdline_page_hostpa)) {
		PANIC("Unable to walk EPT to get Linux cmdline page host physical address");
	}
	char* cmdline_page_hostva = P2V_IDENTITY_MAP(cmdline_page_hostpa);

	/* NOTE : This check is very approximate and has the following identified drawbacks :
	 *        - We assume this is a modern kernel with a pref_address of 0x1000000 (16MiB)
	 *        - We don't take into account the memory used by the decompressor and the early kernel
	 *        - We use the full image size instead of the protected kernel size and don't pad properly the integer divison
	 *        It's intended to PANIC in extreme cases (like loading Linux in a VM with 640KiB of memory), we will PANIC later or the VM will
	 *        triple fault at some point if there's still not enough memory
	 *        It would be nice to improve the check to display a clearer error message to the user in less extreme cases
	 */
	if (vm->guest_physical_pages <= (0x1000000 / PAGE_SIZE) ||
	    vm->guest_physical_pages - vm->guest_physical_pages_used_by_initial_setup - (0x1000000 / PAGE_SIZE) <= (image_size / PAGE_SIZE)) {
		PANIC("Not enough guest physical memory to load the Linux kernel");
	}

	size_t cmdline_len = str_strlen(cmdline);
	if (cmdline_len > image_setup_header->cmdline_size || cmdline_len > PAGE_SIZE) {
		PANIC("Linux cmdline is too long");
	}
	memcpy(cmdline_page_hostva, cmdline, cmdline_len);
	cmdline_page_hostva[cmdline_len] = '\0';

	memset(zeropage_hostva, 0, PAGE_SIZE);
	memcpy(&zeropage_hostva->hdr, image_setup_header, image_setup_header_size);

	const char* image_version_string = (char*)(image + 0x200 + zeropage_hostva->hdr.kernel_version);
	LOG("Loading Linux kernel \"%s\" in vm \"%s\"", image_version_string, vm->name);
	LOG("With kernel command line \"%s\"", cmdline);

	LOG("Using boot protocol version %u.%u", zeropage_hostva->hdr.version >> 8, zeropage_hostva->hdr.version & 0xf);
	if (zeropage_hostva->hdr.version < 0x020f) {
		LOG("WARNING : The Linux guest loader was only tested with boot protocol 2.15 : compatibility is not guaranteed");
	}

	zeropage_hostva->hdr.vid_mode = 0xFFFF;      // vga=normal
	zeropage_hostva->hdr.type_of_loader = 0xFF;  // undefined boot loader
	zeropage_hostva->hdr.loadflags &= 0x1f;      // QUIET_FLAG=0, KEEP_SEGMENTS=0, CAN_USE_HEAP=0
	zeropage_hostva->hdr.heap_end_ptr = 0;       // TODO : setup kernel loader heap, it doesn't seem to be required on modern bzImages

	zeropage_hostva->hdr.ramdisk_image = 0;  // TODO : load initrd
	zeropage_hostva->ext_ramdisk_image = 0;
	zeropage_hostva->hdr.ramdisk_size = 0;
	zeropage_hostva->ext_ramdisk_size = 0;

	zeropage_hostva->hdr.cmd_line_ptr = cmdline_page_guestpa & 0xffffffff;
	zeropage_hostva->ext_cmd_line_ptr = cmdline_page_guestpa >> 32;

	/* NOTE : Linux will not believe a E820 table with only one entry, see append_e820_table in arch/x86/kernel/e820.c
	 *        To overcome this we setup 2 consecutive E820 entries splited at the 640KiB barrier
	 */
	zeropage_hostva->e820_entries = 2;
	zeropage_hostva->e820_table[0].addr = 0;
	zeropage_hostva->e820_table[0].size = 640 * 1024;
	zeropage_hostva->e820_table[0].type = 1;  // E820_TYPE_RAM
	zeropage_hostva->e820_table[1].addr = 0xa0000;
	zeropage_hostva->e820_table[1].size = (vm->guest_physical_pages - 0xa0) * PAGE_SIZE;
	zeropage_hostva->e820_table[1].type = 1;  // E820_TYPE_RAM

	if (zeropage_hostva->hdr.setup_sects == 0) {
		zeropage_hostva->hdr.setup_sects = 4;
	}
	const uint8_t* image_protected_kernel = image + (zeropage_hostva->hdr.setup_sects + 1) * 512;
	size_t image_protected_kernel_size = (image + image_size) - image_protected_kernel;
	size_t image_protected_kernel_pages = (image_protected_kernel_size / PAGE_SIZE) + (image_protected_kernel_size % PAGE_SIZE == 0 ? 0 : 1);

	// TODO : Support custom pref_address and custom alignment for relocatable kernels
	uintptr_t protected_kernel_guestpa = zeropage_hostva->hdr.pref_address;
	if (protected_kernel_guestpa % zeropage_hostva->hdr.kernel_alignment != 0) {
		PANIC("Kernel pref_address isn't aligned according to kernel_alignment");
	}

	for (size_t page = 0; page < image_protected_kernel_pages; page++) {
		uintptr_t page_guestpa = protected_kernel_guestpa + (page * PAGE_SIZE);
		uintptr_t page_hostpa;
		if (!vmx_ept_get_host_physical_address(vm, page_guestpa, &page_hostpa)) {
			PANIC("Unable to walk EPT to get Linux protected kernel host physical address");
		}
		if (page < image_protected_kernel_pages - 1 || image_protected_kernel_size % PAGE_SIZE == 0) {
			memcpy(P2V_IDENTITY_MAP(page_hostpa), image_protected_kernel + (page * PAGE_SIZE), PAGE_SIZE);
		} else {
			memcpy(P2V_IDENTITY_MAP(page_hostpa), image_protected_kernel + (page * PAGE_SIZE), image_protected_kernel_size % PAGE_SIZE);
		}
	}

	vm->initial_gpr_state.rsi = zeropage_guestpa;

	vm->vmcs_config.guest_state.rip = protected_kernel_guestpa + 0x200;  // 64-bit entry point
	vm->vmcs_config.guest_state.rsp = 0;                                 // The kernel loader will setup its own stack
	vm->vmcs_config.guest_state.cr0 = cr0_read();
	vm->vmcs_config.guest_state.cr4 = cr4_read();
	vm->vmcs_config.guest_state.ia32_efer = rdmsr(IA32_EFER);

	// The kernel loader will setup its own GDT and IDT
	vm->vmcs_config.guest_state.cs = RKHV_CS;
	vm->vmcs_config.guest_state.ds = RKHV_DS;
	segments_sgdt(&vm->vmcs_config.guest_state.gdtr);
	interrupts_sidt(&vm->vmcs_config.guest_state.idtr);

	LOG("Linux kernel successfully loaded at guest physical address %zxpq with zeropage at %zxpq", protected_kernel_guestpa, zeropage_guestpa);
}
