#pragma once
#include "vectorintrinsics.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long __mftb();
double __frsqrte(double);

long _InterlockedIncrement(long *lpAddend);
long _InterlockedDecrement(long *lpAddend);

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
