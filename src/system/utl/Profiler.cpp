#include "utl/Profiler.h"
#include "os/Debug.h"

Profiler::Profiler(char const *c, int i)
    : mName(c), mMin(3.4028235e+38), mMax(0.0f), mSum(0.0f), mCount(0), mCountMax(i) {}

void Profiler::Start() { mTimer.Start(); }

void Profiler::Stop() {}
