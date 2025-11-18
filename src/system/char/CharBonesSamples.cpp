#include "char/CharBonesSamples.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"

CharBonesSamples::CharBonesSamples()
    : mNumSamples(0), mPreviewSample(0), mRawData(nullptr) {}

CharBonesSamples::~CharBonesSamples() { MemFree(mRawData); }

int CharBonesSamples::AllocateSize() { return mTotalSize * mNumSamples; }

void CharBonesSamples::RotateBy(CharBones &bones, int i) {
    mStart = &mRawData[mTotalSize * i];
    CharBones::RotateBy(bones);
}

void CharBonesSamples::RotateTo(CharBones &bones, float f1, int i, float f2) {
    mStart = &mRawData[mTotalSize * i];
    CharBones::RotateTo(bones, (1.0f - f2) * f1);
    if (f2 > 0.0f) {
        mStart = &mRawData[mTotalSize * (i + 1)];
        CharBones::RotateTo(bones, f2 * f1);
    }
}

void CharBonesSamples::ScaleAddSample(CharBones &bones, float f1, int i, float f2) {
    mStart = &mRawData[mTotalSize * i];
    CharBones::ScaleAdd(bones, (1.0f - f2) * f1);
    if (f2 > 0.0f) {
        mStart = &mRawData[mTotalSize * (i + 1)];
        CharBones::ScaleAdd(bones, f2 * f1);
    }
}

void CharBonesSamples::ReadCounts(BinStream &bs, int i2) {
    int i = 0;
    int numTypesToRead = Min(7, i2);
    for (; i < numTypesToRead; i++) {
        bs >> mCounts[i];
    }
    for (int numTypesRead = i; numTypesRead < i2; numTypesRead++) {
        int tmp;
        bs >> tmp;
        MILO_ASSERT((tmp - mCounts[NUM_TYPES - 1]) == 0, 0x2af);
    }
    for (; i < 7; i++) {
        mCounts[i] = 0;
    }
}

void CharBonesSamples::Set(
    const std::vector<CharBones::Bone> &bones, int i, CharBones::CompressionType ty
) {
    ClearBones();
    SetCompression(ty);
    mNumSamples = i;
    AddBones(bones);
    MemFree(mRawData);
    mRawData = (char *)MemAlloc(
        AllocateSize(), "CharBonesSamples.cpp", 0x2d, "CharBonesSamples", 0
    );
    mFrames.clear();
}

void CharBonesSamples::Clone(const CharBonesSamples &samp) {
    Set(samp.mBones, samp.mNumSamples, samp.mCompression);
    memcpy(mRawData, samp.mRawData, AllocateSize());
    mFrames = samp.mFrames;
}
