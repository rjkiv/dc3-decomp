#include "synth360/SynthSample.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Memory.h"
#include "synth/SampleData.h"
#include "synth/SampleInst.h"
#include "synth360/SampleInst.h"

void *SampleAlloc(int req, const char *file, int line, const char *, int) {
    void *ret = PhysicalAllocTracked(req, 4, file, line, "SampleData(phys)");
    MILO_ASSERT(ret, 0x19);
    return ret;
}

void SampleFree(void *mem, const char *, int, const char *) {
    if (mem) {
        PhysicalFreeTracked(mem, __FILE__, 0x21, "");
    }
}

void SynthSample360::Init() {
    REGISTER_OBJ_FACTORY(SynthSample360);
    SampleData::SetAllocator(SampleAlloc, SampleFree);
}

int SynthSample360::GetNumSamples() const { return mSampleData.NumSamples(); }
const void *SynthSample360::GetData() const { return mSampleData.GetData(); }
int SynthSample360::GetNumBytes() const { return mSampleData.NumBytes(); }

SampleInst *SynthSample360::NewInst(bool b1, int i2, int i3) {
    if (mSampleData.GetData()) {
        return new SampleInst360(this, b1, i2, i3);
    } else {
        return nullptr;
    }
}

float SynthSample360::LengthMs() const {
    if (mSampleData.GetData()) {
        float numSamples = mSampleData.NumSamples();
        float rate = GetSampleRate();
        return (numSamples * 1000) / rate;
    } else {
        return 0;
    }
}

bool SynthSample360::IsXMA() const { return mSampleData.GetFormat() == SampleData::kXMA; }
