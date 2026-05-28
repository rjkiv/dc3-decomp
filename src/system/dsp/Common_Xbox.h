#pragma once
#include "xdk/XAUDIO2.h"

void DspClearBuffer(float *&, int);
void DspFree(float *&);
void DspAllocate(float *&, int, IXAudioBatchAllocator *);
