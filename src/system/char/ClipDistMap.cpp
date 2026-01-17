#include "char/ClipDistMap.h"
#include "char/CharClip.h"
#include "macros.h"
#include "math/Utl.h"
#include "rndobj/Rnd.h"

void FindWeights(
    std::vector<RndTransformable *> &transes,
    std::vector<float> &floats,
    const DataArray *arr
) {
    floats.resize(transes.size());
    float f1 = 0;
    for (int i = 0; i < transes.size(); i++) {
        float len = Length(transes[i]->LocalXfm().v);
        if (arr) {
            float f84 = 1;
            arr->FindData(transes[i]->Name(), f84, false);
            len *= f84;
        }
        floats[i] = len;
        f1 += floats[i];
    }
    for (int i = 0; i < floats.size(); i++) {
        floats[i] *= floats.size() / f1;
    }
}

DistEntry &DistEntry::operator= (const DistEntry &right) {
    beat = right.beat;
    bones = right.bones;
    for (int i = 0; i < 4; i++) {
        facing[i] = right.facing[i];
    }
    return *this;
}

DistEntry::DistEntry(const DistEntry &entry) : beat(entry.beat), bones(entry.bones) {
    memcpy(facing, entry.facing, sizeof(entry.facing));
}

ClipDistMap::ClipDistMap(
    CharClip *clip1, CharClip *clip2, float f1, float f2, int i, const DataArray *a
)
    : mClipA(clip1), mClipB(clip2), mWeightData(a), mSamplesPerBeat(8),
      mLastMinErr(kHugeFloat), mBeatAlign(f1), mBeatAlignOffset(0), mBlendWidth(f2),
      mNumSamples(i) {
    int h = CalcHeight();
    int w = CalcWidth();
    mDists.Resize(w, h);

    mBeatAlignPeriod = mBeatAlign * mSamplesPerBeat + 0.5;

    int temp;
    if (mBeatAlignPeriod != 0) {
        temp = mAStart * mSamplesPerBeat - mBStart * mSamplesPerBeat;
        mBeatAlignOffset = temp - (temp / mBeatAlignPeriod) * mBeatAlignPeriod;

        if (mBeatAlignOffset < 0) {
            mBeatAlignOffset += mBeatAlignPeriod;
        }
    }
}

bool ClipDistMap::BeatAligned(int i1, int i2) {
    int l1;
    int l2 = mBeatAlignPeriod;

    if (l2 == 0) {
        l1 = 0;
    } else {
        l1 = (i1 - i2) % l2;
        if (l1 < 0) {
            l1 += l2;
        }
    }

    return l1 == mBeatAlignOffset;
}

bool ClipDistMap::FindBestNode(float f1, float f2, float f3, ClipDistMap::Node &node) {
    int temp1;
    int temp2;
    if (f2 < f3) {
        temp1 = (f3 - mAStart) * mSamplesPerBeat;
        temp2 = (f2 - mAStart) * mSamplesPerBeat;
        temp2 = 0xffffffff - (temp2 >> 0x1f) & temp2;
        if (temp1 <= mDists.mWidth) {
            mDists.mWidth = temp1;
        }
        for (int i = 0; i < mDists.mWidth; i++) {
        }
    }
    return false;
}

void ClipDistMap::FindNodes(float f1, float f2, float f3) {
    std::vector<Node> nodes = mNodes;
    mLastMinErr = f1;
    float f5 = f2;
    float f6;
    float f4 = f2 * 0.44999998807907104;
    if (f5 == 0.0) {
        f4 = kHugeFloat;
        f6 = f4;
    } else if (f6 == 0.0) {
        f6 = f4;
    }

    FindBestNodeRecurse(f1, f4, (f4 * 2.0 - f5), mAStart, mAEnd);
    // finish later
}

int ClipDistMap::CalcWidth() {
    float clipAStartBeat = mClipA->StartBeat();
    float samplesDiv = (1.0 / mSamplesPerBeat);
    float clipASamplesMod = Mod(clipAStartBeat, 1.0 / mSamplesPerBeat);
    float f1 = clipAStartBeat - clipASamplesMod;
    mAStart = f1;

    if (f1 < mClipA->StartBeat()) {
        mAStart = f1 + samplesDiv;
    }

    f1 = mClipA->EndBeat();
    clipASamplesMod = Mod(f1, samplesDiv);
    mAEnd = f1 - clipASamplesMod;
    clipASamplesMod = (f1 - clipASamplesMod) + samplesDiv;

    if (clipASamplesMod <= mClipA->EndBeat()) {
        mAEnd = clipASamplesMod;
    }

    f1 = floor(mAEnd - mAStart * mSamplesPerBeat + 0.5);

    uint val = f1;

    return (((val != 0) - (val >> 0x1f) & val)) + 1;
}

int ClipDistMap::CalcHeight() {
    float clipBStartBeat = mClipB->StartBeat();
    float samplesDiv = 1.0f / mSamplesPerBeat;
    float clipBSamplesMod = Mod(clipBStartBeat, samplesDiv);
    float f1 = clipBStartBeat - clipBSamplesMod;
    mBStart = f1;

    if (mBStart < mClipB->StartBeat()) {
        mBStart += samplesDiv;
    }
    // fVar5 = mClipB->EndBeat();
    f1 = mClipB->EndBeat();
    clipBSamplesMod = Mod(mClipB->EndBeat(), samplesDiv);
    clipBStartBeat = (f1 - clipBSamplesMod) + samplesDiv;
    f1 -= clipBSamplesMod;

    if (clipBStartBeat <= mClipB->EndBeat()) {
        f1 = clipBStartBeat;
    }

    f1 = floor(((f1 - mBStart) * (float)mSamplesPerBeat) + 0.5f);
    uint val = f1;

    return (((val != 0) - (val >> 0x1f) & val)) + 1;
}

void ClipDistMap::Array2d::Resize(int w, int h) {
    delete this->mData;
    this->mWidth = w;
    this->mHeight = h;
    this->mData = (float *)new uint[h * w];
}

void ClipDistMap::SetNodes(ClipDistMap::Node *node1, ClipDistMap::Node *node2) {
    mClipB->GetTransitions().RemoveClip(mClipB);
    for (int i = 0; i < mNodes.size(); i++) {
        if (node1) {
            float fVar1 = node1->unk8;
            float fVar2 = mNodes[i].unk8;
            if (node1->unk8 - mNodes[i].unk8 < 0.0) {
                fVar2 = fVar1;
            }
            node1->unk8 = fVar2;
            if (fVar2 != fVar1) {
                node1->unk0 = mNodes[i].unk0;
                node1->unk4 = mNodes[i].unk4;
                node1->unk8 = mNodes[i].unk8;
            }
        }
        if (node2) {
            float fVar1 = node2->unk8;
            float fVar2 = mNodes[i].unk8;
            if (fVar1 - fVar2 < 0.0) {
                fVar2 = fVar1;
            }
            node2->unk8 = fVar2;
            if (fVar2 != fVar1) {
                node2->unk0 = mNodes[i].unk0;
                node2->unk4 = mNodes[i].unk4;
                node2->unk8 = mNodes[i].unk8;
            }
        }
        auto graphNode = CharGraphNode();
        graphNode.nextBeat = mNodes[i].unk4;
        graphNode.curBeat = mNodes[i].unk0;
        mClipB->GetTransitions().AddNode(mClipB, graphNode);
    }
}

void ClipDistMap::DrawDot(float x, float y, float f3, float f4, Hmx::Color const &color) {
    Hmx::Rect rect;
    rect.w = 2.0;
    rect.h = 2.0;
    float scale = (float)mSamplesPerBeat;
    rect.x = (f3 - mAStart) * scale * 2.0f + (x - 1.0f);
    rect.y = ((f4 - mBStart) * scale - (float)(mDists.mHeight - 1)) * 2.0f + y + 1.0f;
    TheRnd.DrawRect(rect, color, nullptr, nullptr, nullptr);
}