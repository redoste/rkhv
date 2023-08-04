#ifndef VM_EMULATED_INSTRUCTIONS
#define VM_EMULATED_INSTRUCTIONS

#include <rkhv/stdint.h>

#include "vmx_vmexit.h"

void vm_emulated_instruction_cpuid(vmx_vmexit_state_t* vm_state);

void vm_emulated_instruction_mov_to_cr(vmx_vmexit_state_t* vm_state, uint8_t crn, uint64_t new_cr);
uint64_t vm_emulated_instruction_mov_from_cr(vmx_vmexit_state_t* vm_state, uint8_t crn);

void vm_emulated_instruction_outb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outd(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_inb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_inw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_ind(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outsb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outsw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_outsd(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_insb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_insw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_insd(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_outsb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_outsw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_outsd(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_insb(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_insw(vmx_vmexit_state_t* vm_state, uint16_t port);
void vm_emulated_instruction_rep_insd(vmx_vmexit_state_t* vm_state, uint16_t port);

void vm_emulated_instruction_xsetbv(vmx_vmexit_state_t* vm_state);

#endif
