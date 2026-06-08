#pragma once
#include "vectorintrinsics.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long __mftb();
double __frsqrte(double);

void __SetHWThreadPriorityLow();
void __SetHWThreadPriorityMed();
void __SetHWThreadPriorityHigh();

// __builtin_constant_p at home
// basically constexpr before constexpr
int __IsIntConst(int);
// i have no idea what the params are, i just know there's 2 of them
void __GPRSetReg(long reg, unsigned long long value);
// i have no idea what the params are, i just know there's 1 of them
unsigned long long __GPRGetReg(long reg);
// atomic fences before atomic fences
void _WriteBarrier();
void _ReadWriteBarrier();
void _ReadBarrier();
// __builtin_return_address at home
void *_ReturnAddress();

// atomics before atomics
long long _InterlockedXor64(long long volatile *Value, long long Mask);
long _InterlockedXor(long volatile *Value, long Mask);
long long _InterlockedOr64(long long volatile *Value, long long Mask);
long _InterlockedOr(long volatile *Value, long Mask);
long long _InterlockedIncrement64(long long volatile *lpAddend);
long _InterlockedIncrement(long volatile *lpAddend);
long long _InterlockedExchangeAdd64(long long volatile *Addend, long long Value);
long _InterlockedExchangeAdd(long volatile *Addend, long Value);
long long _InterlockedExchange64(long long volatile *Target, long long Value);
long _InterlockedExchange(long volatile *Target, long Value);
long long _InterlockedDecrement64(long long volatile *lpAddend);
long _InterlockedDecrement(long volatile *lpAddend);
long long _InterlockedCompareExchange64(
    long long volatile *Destination, long long Exchange, long long Comparand
);
long _InterlockedCompareExchange(
    long volatile *Destination, long Exchange, long Comparand
);
long long _InterlockedAnd64(long long volatile *value, long long mask);
long _InterlockedAnd(long volatile *value, long mask);
unsigned int _CountLeadingZeros64(long long value);
unsigned int _CountLeadingZeros(long value);

#ifdef __cplusplus
}
#endif

// i didn't know where else to put this,
// but the full list of pragmas MSVC supports, for reference:

// auto_inline
// alloc_text
// acp_store
// acp_assume_type
// acp_assume_not_type
// acp_assume_not_defined
// bss_seg
// bitfield_order
// const_seg
// conform
// component
// comment
// code_seg
// const_seg
// conform
// component
// comment
// code_seg
// check_stack
// detect_mismatch
// deprecated
// data_seg
// endregion
// function
// fp_contract
// force_align
// float_control
// fenv_access
// hdrstop
// intrinsic
// inline_recursion
// inline_depth
// init_seg
// include_alias
// implementation_key
// ident
// lint
// message
// managed
// make_public
// optimize
// once
// omp
// push_macro
// push
// prefast
// pop_macro
// pop
// pointers_to_members
// parameter
// pack
// runtime_checks
// reverse_bitfield
// region
// strict_gs_check
// stop_map_region
// start_map_region
// skipping
// setlocale
// segment
// section
// search_lib
// same_seg
// unmanaged
// vtordisp
// warning
