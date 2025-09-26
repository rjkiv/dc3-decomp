#include "char/CharBonesSamples.h"
#include "utl/MemMgr.h"

CharBonesSamples::CharBonesSamples()
    : mNumSamples(0), mPreviewSample(0), mRawData(nullptr) {}

CharBonesSamples::~CharBonesSamples() { MemFree(mRawData); }
