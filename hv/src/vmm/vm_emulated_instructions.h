#ifndef VM_EMULATED_INSTRUCTIONS
#define VM_EMULATED_INSTRUCTIONS

#include <rkhv/stdint.h>

#include "vmx_vmexit.h"

void vm_emulated_instruction_outb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outd(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outsb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outsw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outsd(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_outsb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_outsw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_outsd(vmx_vmexit_state_t* vm_state, uint16_t port);

#endif
