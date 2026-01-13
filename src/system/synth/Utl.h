#pragma once
#include "os/FileCache.h"
#include "utl/Cache.h"
#include "utl/Symbol.h"

void SynthUtlInit();
float CalcRateForTempoSync(Symbol, float);
float CalcSpeedFromTranspose(float);
float CalcTransposeFromSpeed(float);
const char *CacheWav(const char *, CacheResourceResult &);

extern FileCacheHelper *gWavFileCacheHelper;