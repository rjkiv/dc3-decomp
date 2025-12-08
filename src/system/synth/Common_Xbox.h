#pragma once
#include "xdk/xaudio2/xaudio2.h"

void DspClearBuffer(float *&, int);
void DspFree(float *&);
void DspAllocate(float *&, int, IXAudioBatchAllocator *);
