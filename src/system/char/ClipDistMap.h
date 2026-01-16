#pragma once

#include "char/CharBonesMeshes.h"
#include "char/CharClip.h"
#include "char/CharDriver.h"
#include "math/Color.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

struct DistEntry {
public:
    DistEntry(const DistEntry &);
    ~DistEntry() {}

    DistEntry &operator= (const DistEntry &right);

    float beat; // 0x0
    std::vector<Vector3> bones; // 0x4
    float facing[4]; // 0xc
};

class ClipDistMap {
public:
    class Array2d {
    public:
        Array2d() : mWidth(0), mHeight(0), mData(0) {}
        void Resize(int, int);
        int CalcWidth();
        int CalcHeight();
        int Width() { return mWidth; }
        int Height() { return mHeight; }
        float &operator()(int i, int j) { return mData[i + j * mWidth]; }

        int mWidth; // 0x0
        int mHeight; // 0x4
        float *mData; // 0x8
    };

    struct Node {
    public:
        float unk0;
        float unk4;
        float unk8;
    };

    MEM_OVERLOAD(ClipDistMap, 0x24);

    ClipDistMap(CharClip *, CharClip *, float, float, int, DataArray const *);
    void SetNodes(ClipDistMap::Node *, ClipDistMap::Node *);
    void Draw(float, float, CharDriver *);
    void FindNodes(float, float, float);
    void FindDists(float, DataArray *);
    CharClip *ClipA() { return mClipA; }
    CharClip *ClipB() { return mClipB; }

    CharClip *mClipA; // 0x0
    CharClip *mClipB; // 0x4
    const DataArray *mWeightData; // 0x8
    float mAStart; // 0xc
    float mAEnd; // 0x10
    float mBStart; // 0x14
    int mSamplesPerBeat; // 0x18
    float mWorstErr; // 0x1c
    float mLastMinErr; // 0x20
    float mBeatAlign; // 0x24
    int mBeatAlignOffset; // 0x28
    int mBeatAlignPeriod; // 0x2c
    float mBlendWidth; // 0x30
    int mNumSamples; // 0x34
    Array2d mDists; // 0x38
    std::vector<Node> mNodes; // 0x44

protected:
    bool BeatAligned(int, int);
    void DrawDot(float, float, float, float, Hmx::Color const &);
    bool LocalMin(int, int);
    int CalcWidth();
    int CalcHeight();
    bool FindBestNode(float, float, float, ClipDistMap::Node &);
    void FindBestNodeRecurse(float, float, float, float, float);
    void GenerateDistEntry(
        CharBonesMeshes &,
        DistEntry &,
        float,
        CharClip *,
        const std::vector<RndTransformable *> &
    );
};
