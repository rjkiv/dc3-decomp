#include "char/CharBonesSamples.h"

#include "CharClip.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"

CharBonesSamples::CharBonesSamples()
    : mNumSamples(0), mPreviewSample(0), mRawData(nullptr) {}

CharBonesSamples::~CharBonesSamples() { MemFree(mRawData); }

BEGIN_LOADS(CharBonesSamples)
LOAD_REVS(bs)
MILO_FAIL("%s can\'t load new %s version %d > %d", "", "ChaBonesSample", bs.Tell(), (unsigned short)1);
LoadHeader(d);
LoadData(d);
END_LOADS

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

void CharBonesSamples::Print() {
    auto samples = mNumSamples;
    auto size = mTotalSize * mNumSamples;
    auto address = mRawData;
    auto compression = mCompression;
    MILO_LOG("samples: %d size: %d address: %x compression %d\n", samples, size, address, compression);
    if (mNumSamples == 0) {
        TheDebug << "Bones:\n";
        for (int i = 0; i < mBones.size(); i++) {
            TheDebug << "   " << mBones[i].name << "\n";
        }
    }
    for (int i = 0; i < mNumSamples; i++) {
        TheDebug << i << ")\n";
        mStart = mRawData + mTotalSize * i;
        CharBones::Print();
    }
}

void CharBonesSamples::Relativize(CharClip *clip) {
    if (mBones.empty())
        return;

    for (int sample = mNumSamples - 1; sample >= 0; sample--) {
        mStart = mRawData + sample * mTotalSize;
        float startBeat = clip->StartBeat();
        int boneIdx = 0;
        if (mCompression < kCompressVects) {
            //ShortVector3 *pos =
        }
    }
}

int CharBonesSamples::FracToSample(float *frac) const {
    if (mNumSamples < 2) {
        *frac = 0.0f;
        return 0;
    }
    float inputFrac = *frac;
    float clampedFrac = Clamp(0.0f, 1.0f, inputFrac);
    *frac = clampedFrac;
    int total = Max((int)mFrames.size(), mNumSamples);
    float scaledPos = clampedFrac * (total - 1);
    *frac = scaledPos;
    int sampleIdx = scaledPos;
    if (sampleIdx >= total - 1) {
        *frac = 0.0f;
        return mNumSamples - 1;
    }
    float interpFrac = scaledPos - sampleIdx;
    *frac = interpFrac;
    int ret = sampleIdx;
    if (mFrames.size() != 0) { // sometimes accessing mFrames at 0x50? wtf is going on
        float frame = mFrames[sampleIdx];
        float nextFrame = mFrames[sampleIdx + 1];
        float interpFrame = frame + (nextFrame - frame) * interpFrac;
        ret = interpFrame;
        *frac = interpFrame - ret;
    }
    if (ret < 0 || ret >= mNumSamples) {
        MILO_NOTIFY_ONCE(
            "FracToSample: sample is %d, clip only has %d samples, frac was %g, is %g",
            ret, mNumSamples, inputFrac, *frac
        );
        ret = 0;
    }
    if (*frac < 0.0f || *frac >= 1.0f) {
        MILO_NOTIFY_ONCE("FracToSample: frac is %g, outside of 0 and 1", *frac);
        *frac = 0.0f;
    }
    return ret;
}