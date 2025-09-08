#pragma once
#include "char/CharBones.h"

class CharBonesSamples : public CharBones {
public:
    CharBonesSamples();
    virtual ~CharBonesSamples();
    virtual void Print();
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

protected:
    /** "how many keyframes" */
    int mNumSamples; // 0x54
    /** "which sample to preview" */
    int mPreviewSample; // 0x58
    char *mRawData; // 0x5c
    /** "which sample to play" */
    std::vector<float> mFrames; // 0x60
};
