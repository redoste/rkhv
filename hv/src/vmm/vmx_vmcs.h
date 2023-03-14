#ifndef VMX_VMCS
#define VMX_VMCS

#include <rkhv/stdint.h>

#define VMCS_CF_VPID                                                  0x00000000
#define VMCS_CF_POSTED_INTERRUPT_NOTIFICATION_VECTOR                  0x00000002
#define VMCS_CF_EPTP_INDEX                                            0x00000004
#define VMCS_CF_HLAT_PREFIX_SIZE                                      0x00000006
#define VMCS_CF_LAST_PID_POINTER_INDEX                                0x00000008
#define VMCS_CF_ADDRESS_OF_MSR_BITMAPS                                0x00002004
#define VMCS_CF_VM_EXIT_MSR_STORE_ADDRESS                             0x00002006
#define VMCS_CF_VM_EXIT_MSR_LOAD_ADDRESS                              0x00002008
#define VMCS_CF_VM_ENTRY_MSR_LOAD_ADDRESS                             0x0000200A
#define VMCS_CF_EXECUTIVE_VMCS_POINTER                                0x0000200C
#define VMCS_CF_PML_ADDRESS                                           0x0000200E
#define VMCS_CF_TSC_OFFSET                                            0x00002010
#define VMCS_CF_VIRTUAL_APIC_ADDRESS                                  0x00002012
#define VMCS_CF_APIC_ACCESS_ADDRESS                                   0x00002014
#define VMCS_CF_POSTED_INTERRUPT_DESCRIPTOR_ADDRESS                   0x00002016
#define VMCS_CF_VM_FUNCTION_CONTROLS                                  0x00002018
#define VMCS_CF_EPT_POINTER                                           0x0000201A
#define VMCS_CF_EOI_EXIT_BITMAP_0                                     0x0000201C
#define VMCS_CF_EOI_EXIT_BITMAP_1                                     0x0000201E
#define VMCS_CF_EOI_EXIT_BITMAP_2                                     0x00002020
#define VMCS_CF_EOI_EXIT_BITMAP_3                                     0x00002022
#define VMCS_CF_EPTP_LIST_ADDRESS                                     0x00002024
#define VMCS_CF_VMREAD_BITMAP_ADDRESS                                 0x00002026
#define VMCS_CF_VMWRITE_BITMAP_ADDRESS                                0x00002028
#define VMCS_CF_VIRTUALIZATION_EXCEPTION_INFORMATION_ADDRESS          0x0000202A
#define VMCS_CF_XSS_EXITING_BITMAP                                    0x0000202C
#define VMCS_CF_ENCLS_EXITING_BITMAP                                  0x0000202E
#define VMCS_CF_SUB_PAGE_PERMISSION_TABLE_POINTER                     0x00002030
#define VMCS_CF_TSC_MULTIPLIER                                        0x00002032
#define VMCS_CF_TERTIARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS        0x00002034
#define VMCS_CF_ENCLV_EXITING_BITMAP                                  0x00002036
#define VMCS_CF_LOW_PASID_DIRECTORY_ADDRESS                           0x00002038
#define VMCS_CF_HIGH_PASID_DIRECTORY_ADDRESS                          0x0000203A
#define VMCS_CF_SHARED_EPT_POINTER                                    0x0000203C
#define VMCS_CF_PCONFIG_EXITING_BITMAP                                0x0000203E
#define VMCS_CF_HYPERVISOR_MANAGED_LINEAR_ADDRESS_TRANSLATION_POINTER 0x00002040
#define VMCS_CF_PID_POINTER_TABLE_ADDRESS                             0x00002042
#define VMCS_CF_SECONDARY_VM_EXIT_CONTROLS                            0x00002044
#define VMCS_CF_PIN_BASED_VM_EXECUTION_CONTROLS                       0x00004000
#define VMCS_CF_PRIMARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS         0x00004002
#define VMCS_CF_EXCEPTION_BITMAP                                      0x00004004
#define VMCS_CF_PAGE_FAULT_ERROR_CODE_MASK                            0x00004006
#define VMCS_CF_PAGE_FAULT_ERROR_CODE_MATCH                           0x00004008
#define VMCS_CF_CR3_TARGET_COUNT                                      0x0000400A
#define VMCS_CF_PRIMARY_VM_EXIT_CONTROLS                              0x0000400C
#define VMCS_CF_VM_EXIT_MSR_STORE_COUNT                               0x0000400E
#define VMCS_CF_VM_EXIT_MSR_LOAD_COUNT                                0x00004010
#define VMCS_CF_VM_ENTRY_CONTROLS                                     0x00004012
#define VMCS_CF_VM_ENTRY_MSR_LOAD_COUNT                               0x00004014
#define VMCS_CF_VM_ENTRY_INTERRUPTION_INFORMATION_FIELD               0x00004016
#define VMCS_CF_VM_ENTRY_EXCEPTION_ERROR_CODE                         0x00004018
#define VMCS_CF_VM_ENTRY_INSTRUCTION_LENGTH                           0x0000401A
#define VMCS_CF_TPR_THRESHOLD                                         0x0000401C
#define VMCS_CF_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS       0x0000401E
#define VMCS_CF_PLE_GAP                                               0x00004020
#define VMCS_CF_PLE_WINDOW                                            0x00004022
#define VMCS_CF_CR0_GUEST_HOST_MASK                                   0x00006000
#define VMCS_CF_CR4_GUEST_HOST_MASK                                   0x00006002
#define VMCS_CF_CR0_READ_SHADOW                                       0x00006004
#define VMCS_CF_CR4_READ_SHADOW                                       0x00006006
#define VMCS_CF_CR3_TARGET_VALUE_0                                    0x00006008
#define VMCS_CF_CR3_TARGET_VALUE_1                                    0x0000600A
#define VMCS_CF_CR3_TARGET_VALUE_2                                    0x0000600C
#define VMCS_CF_CR3_TARGET_VALUE_3                                    0x0000600E

/* Primary Processor-Based VM-Execution Controls bits : Vol 3 Table 25-6 */
#define VMCS_CF_PPBVMEC_HLT_EXITING                 (1 << 7)
#define VMCS_CF_PPBVMEC_UNCONDITIONAL_IO_EXITING    (1 << 24)
#define VMCS_CF_PPBVMEC_ACTIVATE_SECONDARY_CONTROLS (1 << 31)

/* VM-Exit Controls bits : Vol 3 Table 25-13 */
#define VMCS_CF_VMEX_HOST_ADDRESS_SPACE_SIZE (1 << 9)

/* VM-Entry Controls bits : Vol 3 Table 25-15 */
#define VMCS_CF_VMEN_IA_32E_MODE_GUEST (1 << 9)

#define VMCS_HOST_ES                             0x00000C00
#define VMCS_HOST_CS                             0x00000C02
#define VMCS_HOST_SS                             0x00000C04
#define VMCS_HOST_DS                             0x00000C06
#define VMCS_HOST_FS                             0x00000C08
#define VMCS_HOST_GS                             0x00000C0A
#define VMCS_HOST_TR                             0x00000C0C
#define VMCS_HOST_CR0                            0x00006C00
#define VMCS_HOST_CR3                            0x00006C02
#define VMCS_HOST_CR4                            0x00006C04
#define VMCS_HOST_FS_BASE                        0x00006C06
#define VMCS_HOST_GS_BASE                        0x00006C08
#define VMCS_HOST_TR_BASE                        0x00006C0A
#define VMCS_HOST_GDTR_BASE                      0x00006C0C
#define VMCS_HOST_IDTR_BASE                      0x00006C0E
#define VMCS_HOST_IA32_SYSENTER_ESP              0x00006C10
#define VMCS_HOST_IA32_SYSENTER_EIP              0x00006C12
#define VMCS_HOST_RSP                            0x00006C14
#define VMCS_HOST_RIP                            0x00006C16
#define VMCS_HOST_IA32_S_CET                     0x00006C18
#define VMCS_HOST_SSP1                           0x00006C1A
#define VMCS_HOST_IA32_INTERRUPT_SSP_TABLE_ADDR1 0x00006C1C

#define VMCS_GUEST_ES                            0x00000800
#define VMCS_GUEST_CS                            0x00000802
#define VMCS_GUEST_SS                            0x00000804
#define VMCS_GUEST_DS                            0x00000806
#define VMCS_GUEST_FS                            0x00000808
#define VMCS_GUEST_GS                            0x0000080A
#define VMCS_GUEST_LDTR                          0x0000080C
#define VMCS_GUEST_TR                            0x0000080E
#define VMCS_GUEST_INTERRUPT_STATUS              0x00000810
#define VMCS_GUEST_PML_INDEX                     0x00000812
#define VMCS_GUEST_UINV                          0x00000814
#define VMCS_GUEST_VMCS_LINK_POINTER             0x00002800
#define VMCS_GUEST_IA32_DEBUGCTL                 0x00002802
#define VMCS_GUEST_IA32_EFER                     0x00002806
#define VMCS_GUEST_ES_LIMIT                      0x00004800
#define VMCS_GUEST_CS_LIMIT                      0x00004802
#define VMCS_GUEST_SS_LIMIT                      0x00004804
#define VMCS_GUEST_DS_LIMIT                      0x00004806
#define VMCS_GUEST_FS_LIMIT                      0x00004808
#define VMCS_GUEST_GS_LIMIT                      0x0000480A
#define VMCS_GUEST_LDTR_LIMIT                    0x0000480C
#define VMCS_GUEST_TR_LIMIT                      0x0000480E
#define VMCS_GUEST_GDTR_LIMIT                    0x00004810
#define VMCS_GUEST_IDTR_LIMIT                    0x00004812
#define VMCS_GUEST_ES_ACCESS_RIGHTS              0x00004814
#define VMCS_GUEST_CS_ACCESS_RIGHTS              0x00004816
#define VMCS_GUEST_SS_ACCESS_RIGHTS              0x00004818
#define VMCS_GUEST_DS_ACCESS_RIGHTS              0x0000481A
#define VMCS_GUEST_FS_ACCESS_RIGHTS              0x0000481C
#define VMCS_GUEST_GS_ACCESS_RIGHTS              0x0000481E
#define VMCS_GUEST_LDTR_ACCESS_RIGHTS            0x00004820
#define VMCS_GUEST_TR_ACCESS_RIGHTS              0x00004822
#define VMCS_GUEST_INTERRUPTIBILITY_STATE        0x00004824
#define VMCS_GUEST_ACTIVITY_STATE                0x00004826
#define VMCS_GUEST_SMBASE                        0x00004828
#define VMCS_GUEST_IA32_SYSENTER_CS              0x0000482A
#define VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE    0x0000482E
#define VMCS_GUEST_CR0                           0x00006800
#define VMCS_GUEST_CR3                           0x00006802
#define VMCS_GUEST_CR4                           0x00006804
#define VMCS_GUEST_ES_BASE                       0x00006806
#define VMCS_GUEST_CS_BASE                       0x00006808
#define VMCS_GUEST_SS_BASE                       0x0000680A
#define VMCS_GUEST_DS_BASE                       0x0000680C
#define VMCS_GUEST_FS_BASE                       0x0000680E
#define VMCS_GUEST_GS_BASE                       0x00006810
#define VMCS_GUEST_LDTR_BASE                     0x00006812
#define VMCS_GUEST_TR_BASE                       0x00006814
#define VMCS_GUEST_GDTR_BASE                     0x00006816
#define VMCS_GUEST_IDTR_BASE                     0x00006818
#define VMCS_GUEST_DR7                           0x0000681A
#define VMCS_GUEST_RSP                           0x0000681C
#define VMCS_GUEST_RIP                           0x0000681E
#define VMCS_GUEST_RFLAGS                        0x00006820
#define VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS      0x00006822
#define VMCS_GUEST_IA32_SYSENTER_ESP             0x00006824
#define VMCS_GUEST_IA32_SYSENTER_EIP             0x00006826
#define VMCS_GUEST_IA32_S_CET                    0x00006828
#define VMCS_GUEST_SSP                           0x0000682A
#define VMCS_GUEST_IA32_INTERRUPT_SSP_TABLE_ADDR 0x0000682C

#define VMCS_INVALID_LINK_POINTER 0xffffffffffffffff

#define VMCS_UNUSABLE_SEGMENT (1 << 16)

#define VMCS_EXIT_REASON                0x00004402
#define VMCS_VM_EXIT_INSTRUCTION_LENGTH 0x0000440C
#define VMCS_EXIT_QUALIFICATION         0x00006400

uintptr_t vmx_create_initialized_vmcs(void);

#endif
