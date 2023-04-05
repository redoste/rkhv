#ifndef COMMON_RKHV_ASSERT_H
#define COMMON_RKHV_ASSERT_H

#ifdef RKHV_BOOT
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#define static_assert _Static_assert

#ifdef RKHV_BOOT
#pragma clang diagnostic pop
#endif

#endif
