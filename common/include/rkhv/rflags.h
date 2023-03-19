#ifndef COMMON_RKHV_RFLAGS
#define COMMON_RKHV_RFLAGS

#define RFLAGS_CF       (1 << 0)
#define RFLAGS_RESERVED (1 << 1)
#define RFLAGS_PF       (1 << 2)
#define RFLAGS_AF       (1 << 4)
#define RFLAGS_ZF       (1 << 6)
#define RFLAGS_SF       (1 << 7)
#define RFLAGS_TF       (1 << 8)
#define RFLAGS_IF       (1 << 9)
#define RFLAGS_DF       (1 << 10)
#define RFLAGS_OF       (1 << 11)
#define RFLAGS_IOPL     (3 << 12)
#define RFLAGS_NT       (1 << 14)
#define RFLAGS_RF       (1 << 16)
#define RFLAGS_VM       (1 << 17)
#define RFLAGS_AC       (1 << 18)
#define RFLAGS_VIF      (1 << 19)
#define RFLAGS_VIP      (1 << 20)
#define RFLAGS_ID       (1 << 21)

#endif
