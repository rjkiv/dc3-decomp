#pragma once
// it's going here in LIBCMT cuz idk where else to put it

#if !defined(_M_PPC)
#define _M_PPC 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma intrinsic(__mftb)
// #pragma intrinsic(__mftb32)

unsigned long long __mftb();

// // Return the 64-bit time base register
// __forceinline unsigned __int64 __mftb(void) {
//     unsigned int hi, lo;
//     __asm {
//         mftbu hi
//         mftb  lo
//         mftbu r0
//         cmpw  hi, r0
//         bne   $-8
//     }
//     return ((unsigned __int64)hi << 32) | lo;
// }

// Return the lower 32 bits of the time base register
// __forceinline unsigned __int32 __mftb32(void) {
//     unsigned int lo;
//     __asm {
//         mftb lo
//     }
//     return lo;
// }

#ifdef __cplusplus
}
#endif
