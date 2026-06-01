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
