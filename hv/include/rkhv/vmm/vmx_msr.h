#ifndef COMMON_VMX_MSR
#define COMMON_VMX_MSR

#include <rkhv/stdint.h>

void vmx_msr_init_bitmaps(uint8_t* msr_bitmaps);

#endif
