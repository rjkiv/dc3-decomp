#pragma once
#include "obj/Object.h"
#include "synth/SynthSample.h"

class SynthSample360 : public SynthSample {
public:
    SynthSample360() {}
    OBJ_CLASSNAME(SynthSample);
    OBJ_SET_TYPE(SynthSample);
    // SynthSample
    virtual SampleInst *NewInst(bool, int, int);
    virtual float LengthMs() const;

    NEW_OBJ(SynthSample360);
    static void Init();

    int GetNumSamples() const;
    bool IsXMA() const;
    const void *GetData() const;
    int GetNumBytes() const;

private:
};
