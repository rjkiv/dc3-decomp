#pragma once
#include "char/CharBonesMeshes.h"
#include "char/CharClip.h"
#include "char/CharDriver.h"
#include "math/Color.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

struct DistEntry {
    float beat; // 0x0
    std::vector<Vector3> bones; // 0x4
    float facing[4]; // 0x10
};

class ClipDistMap {
public:
    class Array2d {
    public:
        Array2d(int w, int h) : mWidth(0), mHeight(0), mData(0) { Resize(w, h); }
        ~Array2d() { delete[] mData; }
        void Resize(int w, int h);
        int Width() { return mWidth; }
        int Height() { return mHeight; }
        float &operator()(int i, int j) { return mData[i + j * mWidth]; }

    protected:
        int mWidth; // 0x0
        int mHeight; // 0x4
        float *mData; // 0x8
    };

    struct Node {
        float a; // 0x0
        float b; // 0x4
        float err; // 0x8
    };

    // yep, hmx actually had this name
    // musta been a copy/paste error
    MEM_OVERLOAD(ShadowBone, 0x24);

    ClipDistMap(
        CharClip *a,
        CharClip *b,
        float beatAlign,
        float blendWidth,
        int numSamples,
        const DataArray *weightData
    );
    void SetNodes(ClipDistMap::Node *best, ClipDistMap::Node *worst);
    void Draw(float x, float y, CharDriver *d);
    void FindNodes(float minErr, float maxDist, float endDist);
    void FindDists(float maxFacing, DataArray *restricts);
    CharClip *ClipA() const { return mClipA; }
    CharClip *ClipB() const { return mClipB; }
    std::vector<Node> &Nodes() { return mNodes; }
    // void PrintNodes();

protected:
    bool BeatAligned(int i, int j);
    void DrawDot(float x, float y, float beatA, float beatB, const Hmx::Color &col);
    bool LocalMin(int x, int y);
    int CalcWidth();
    int CalcHeight();
    bool FindBestNode(float minErr, float a, float b, ClipDistMap::Node &n);
    void
    FindBestNodeRecurse(float minErr, float minDist, float minGap, float start, float end);
    void GenerateDistEntry(
        CharBonesMeshes &boneMeshes,
        DistEntry &e,
        float beat,
        CharClip *clip,
        const std::vector<RndTransformable *> &bones
    );
    float BeatA(int);
    float BeatB(int);
    float IndexA(float);
    float IndexB(float);

    // protected: bool ClipLine(Vector2&, Vector2&, float);
    // protected: float ClipBeat(CharClip*, CharDriver*, bool);

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
};
