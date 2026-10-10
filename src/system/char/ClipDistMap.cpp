#include "char/ClipDistMap.h"
#include "char/CharBoneDir.h"
#include "char/CharBonesMeshes.h"
#include "char/CharClip.h"
#include "char/CharUtl.h"
#include "math/Trig.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "os/Debug.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"
#include "string.h"
#include "utl/Std.h"
#include <cmath>

static const float sLargeFloat = kHugeFloat;

struct DistMapNodeSort {
    bool operator()(const ClipDistMap::Node &n1, const ClipDistMap::Node &n2) const {
        return n1.a < n2.a;
    }
};

void FindWeights(
    std::vector<RndTransformable *> &bones,
    std::vector<float> &weights,
    const DataArray *weightData
) {
    weights.resize(bones.size());
    float f1 = 0;
    for (int i = 0; i < bones.size(); i++) {
        float len = Length(bones[i]->LocalXfm().v);
        if (weightData) {
            float f84 = 1;
            weightData->FindData(bones[i]->Name(), f84, false);
            len *= f84;
        }
        weights[i] = len;
        f1 += weights[i];
    }
    for (int i = 0; i < weights.size(); i++) {
        weights[i] *= weights.size() / f1;
    }
}

#pragma region Array2d

void ClipDistMap::Array2d::Resize(int w, int h) {
    delete mData;
    mWidth = w;
    mHeight = h;
    mData = new float[w * h];
}

#pragma endregion

ClipDistMap::ClipDistMap(
    CharClip *clip1, CharClip *clip2, float f1, float f2, int i, const DataArray *a
)
    : mClipA(clip1), mClipB(clip2), mWeightData(a), mSamplesPerBeat(8),
      mLastMinErr(sLargeFloat), mBeatAlign(f1), mBeatAlignOffset(0), mBlendWidth(f2),
      mNumSamples(i), mDists(CalcWidth(), CalcHeight()) {
    mBeatAlignPeriod = mBeatAlign * (float)mSamplesPerBeat + 0.5;
    if (mBeatAlignPeriod != 0) {
        int tmp = (-mAStart - -mBStart) * (float)mSamplesPerBeat;
        mBeatAlignOffset = Offset(tmp);
    }
}

bool ClipDistMap::BeatAligned(int i1, int i2) {
    return Offset(i1 - i2) == mBeatAlignOffset;
}

bool ClipDistMap::FindBestNode(float f1, float f2, float f3, ClipDistMap::Node &node) {
    if (f2 >= f3) {
        return false;
    } else {
        node.err = f1;
        int distsWidth = mDists.Width();
        int i1 = (f3 - mAStart) * (float)mSamplesPerBeat;
        int i2 = (f2 - mAStart) * (float)mSamplesPerBeat;
        int i = Max(i2, 0);
        distsWidth = Max(i1, distsWidth);
        for (; i < distsWidth; i++) {
            float samps = mSamplesPerBeat;
            int j = mDists.Height();
            int minus = j - 1;
            if (minus >= 0) {
                for (; j != 0; j--, minus--) {
                    if (MinEq(node.err, mDists(i, minus))) {
                        node.a = (float)i / samps + mAStart;
                        node.b = (float)minus / samps + mBStart;
                    }
                }
            }
        }
        return node.err < f1;
    }
}

void ClipDistMap::FindNodes(float minErr, float maxDist, float endDist) {
    mNodes.clear();
    mLastMinErr = minErr;

    float f7 = maxDist * 0.45f;
    if (maxDist == 0) {
        f7 = sLargeFloat;
        endDist = f7;
    } else if (endDist == 0) {
        endDist = maxDist;
    }

    FindBestNodeRecurse(minErr, f7, -(f7 * 2.0f - maxDist), mAStart, mAEnd);
    std::sort(mNodes.begin(), mNodes.end(), DistMapNodeSort());
    if (!mNodes.empty() && endDist > 0) {
        if (mAEnd - mNodes.back().a > endDist) {
            Node node;
            if (FindBestNode(minErr, mAEnd - endDist, mAEnd, node)) {
                mNodes.push_back(node);
                std::sort(mNodes.begin(), mNodes.end(), DistMapNodeSort());
            }
        }
    }
    for (int i = 1; i < (int)mNodes.size() - 1; i++) {
        if (mNodes[i + 1].a - mNodes[i - 1].a < maxDist) {
            mNodes.erase(mNodes.begin() + i);
            i--;
        }
    }
}

int ClipDistMap::CalcWidth() {
    float clipAStartBeat = mClipA->StartBeat();
    float div = 1.0f / (float)mSamplesPerBeat;
    mAStart = clipAStartBeat - Mod(clipAStartBeat, div);
    if (mAStart < mClipA->StartBeat()) {
        mAStart += div;
    }
    float clipAEndBeat = mClipA->EndBeat();
    mAEnd = clipAEndBeat - Mod(clipAEndBeat, div);
    if (mAEnd + div <= mClipA->EndBeat()) {
        mAEnd += div;
    }
    int i1 = Max(0, (int)floorf((mAEnd - mAStart) * (float)(mSamplesPerBeat) + 0.5f)) + 1;
    mAEnd = (float)(i1 - 1) / (float)mSamplesPerBeat + mAStart;
    return i1;
}

int ClipDistMap::CalcHeight() {
    float clipBStartBeat = mClipB->StartBeat();
    float div = 1.0f / (float)mSamplesPerBeat;
    mBStart = clipBStartBeat - Mod(clipBStartBeat, div);
    if (mBStart < mClipB->StartBeat()) {
        mBStart += div;
    }
    float clipBEndBeat = mClipB->EndBeat();
    float bEnd = clipBEndBeat - Mod(clipBEndBeat, div);
    if (bEnd + div <= mClipB->EndBeat()) {
        bEnd += div;
    }
    return Max(0, (int)floorf((bEnd - mBStart) * (float)(mSamplesPerBeat) + 0.5f)) + 1;
}

void ClipDistMap::SetNodes(ClipDistMap::Node *best, ClipDistMap::Node *worst) {
    mClipA->GetTransitions().RemoveClip(mClipB);
    for (int i = 0; i < mNodes.size(); i++) {
        if (best && MinEq(best->err, mNodes[i].err)) {
            *best = mNodes[i];
        }
        if (worst && MaxEq(worst->err, mNodes[i].err)) {
            *worst = mNodes[i];
        }
        float a = mNodes[i].a;
        float b = mNodes[i].b;
        CharGraphNode graphNode;
        graphNode.curBeat = a;
        graphNode.nextBeat = b;
        mClipA->GetTransitions().AddNode(mClipB, graphNode);
    }
}

void ClipDistMap::DrawDot(float x, float y, float f3, float f4, Hmx::Color const &color) {
    Hmx::Rect rect;
    rect.w = 2.0;
    rect.h = 2.0;
    float scale = (float)mSamplesPerBeat;
    rect.x = (f3 - mAStart) * scale * 2.0f + (x - 1.0f);
    rect.y = ((float)(mDists.Height() - 1) - (f4 - mBStart) * scale) * 2.0f + y + 1.0f;
    TheRnd.DrawRect(rect, color, nullptr, nullptr, nullptr);
}

bool ClipDistMap::LocalMin(int i, int j) {
    float data = mDists(i, j);
    if (data == sLargeFloat) {
        return false;
    } else {
        if (mBeatAlign != 0 && BeatAligned(i, j)) {
            if (i - 1 >= 0 && j - 1 >= 0 && mDists(i - 1, j - 1) < data) {
                return false;
            }
            if (i + 1 < mDists.Width() && j + 1 < mDists.Height()
                && mDists(i + 1, j + 1) < data) {
                return false;
            }
            return true;
        }

        for (int n = i - 1; n < i + 2; n++) {
            for (int i9 = j - 1; i9 < j + 2; i9++) {
                if (n != i || i9 != j) {
                    if (n >= 0 && n < mDists.Width() && i9 >= 0 && i9 < mDists.Height()) {
                        float curData = mDists(n, i9);
                        if (curData != sLargeFloat && curData < data) {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }
}

void ClipDistMap::FindBestNodeRecurse(
    float minErr, float minDist, float minGap, float start, float end
) {
    MILO_ASSERT(minDist > 0, 0x26C);
    if ((end - start) > minGap) {
        float f10 = Max(end, minDist + start + minGap);
        float f9 = Min(start, (end - minGap) - minDist);
        Node n;
        if (FindBestNode(minErr, f9, f10, n)) {
            for (int i = 0; i < mNodes.size(); i++) {
                if (mNodes[i].a == n.a) {
                    return;
                }
            }
            mNodes.push_back(n);
            FindBestNodeRecurse(minErr, minDist, minGap, n.a + minDist, end);
            FindBestNodeRecurse(minErr, minDist, minGap, start, n.a - minDist);
        }
    }
}

void ClipDistMap::GenerateDistEntry(
    CharBonesMeshes &boneMeshes,
    DistEntry &e,
    float beat,
    CharClip *clip,
    const std::vector<RndTransformable *> &bones
) {
    if (e.bones.empty()) {
        e.beat = beat;
        float blendWidth = mBlendWidth / 4.0f;
        float f14 = blendWidth / 2.0f + beat;
        void *facingChannel = clip->GetChannel("bone_facing.rotz");
        for (int i = 0; i < 4; i++, f14 += blendWidth) {
            e.facing[i] = 0;
            if (facingChannel) {
                clip->EvaluateChannel(&e.facing[i], facingChannel, f14);
            }
            e.facing[i] = LimitAng(e.facing[i]);
        }
        e.bones.resize(bones.size() * mNumSamples);
        int eBonesIdx = 0;
        blendWidth = mBlendWidth / (float)mNumSamples;
        f14 = blendWidth / 2.0f + e.beat;
        for (int i = 0; i < mNumSamples; i++, f14 += blendWidth) {
            CharUtlResetTransform(clip->GetResource());
            boneMeshes.Zero();
            clip->ScaleAdd(boneMeshes, 1, f14, 0);
            boneMeshes.PoseMeshes();
            for (int j = 0; j < bones.size(); j++) {
                e.bones[eBonesIdx++] = bones[j]->WorldXfm().v;
            }
        }
    }
}

float ClipDistMap::BeatA(int i) { return (float)(mAStart + i) / (float)mSamplesPerBeat; }
float ClipDistMap::BeatB(int i) { return (float)(mBStart + i) / (float)mSamplesPerBeat; }

void ClipDistMap::FindDists(float maxFacing, DataArray *restricts) {
    CharBoneDir *rsrcDir = mClipA->GetResource();
    CharUtlBoneSaver saver(rsrcDir);
    CharBonesMeshes meshes;
    meshes.SetName("tmp_bones", rsrcDir);
    rsrcDir->StuffBones(meshes, mClipA->GetContext());
    std::vector<RndTransformable *> bones;
    for (ObjDirItr<RndTransformable> it(rsrcDir, true); it != nullptr; ++it) {
        if (strnicmp(it->Name(), "bone", 4) == 0) {
            bones.push_back(&*it);
        }
    }
    mClipA->GetChannel("bone_facing.rotz");
    DataNode &aBeatVar = DataVariable("a_beat");
    float aBeatVal = aBeatVar.Float();
    DataNode &bBeatVar = DataVariable("b_beat");
    float bBeatVal = bBeatVar.Float();
    DataNode &aStartVar = DataVariable("a_start");
    float aStartVal = aStartVar.Float();
    DataNode &aEndVar = DataVariable("a_end");
    float aEndVal = aEndVar.Float();
    DataNode &bStartVar = DataVariable("b_start");
    float bStartVal = bStartVar.Float();
    DataNode &bEndVar = DataVariable("b_end");
    float bEndVal = bEndVar.Float();
    DataNode &aMiddleVar = DataVariable("a_middle");
    float aMiddleVal = aMiddleVar.Float();
    DataNode &bMiddleVar = DataVariable("b_middle");
    float bMiddleVal = bMiddleVar.Float();
    DataNode &deltaVar = DataVariable("delta");
    float deltaVal = deltaVar.Float();
    std::vector<DistEntry> distEntries;
    distEntries.resize(mDists.Height());
    std::vector<float> weights;
    float aInterp = Interp(mClipA->StartBeat(), mClipA->EndBeat(), 0.5f);
    float bInterp = Interp(mClipB->StartBeat(), mClipB->EndBeat(), 0.5f);
    mWorstErr = 0;
    for (int i = 0; i < mDists.Width(); i++) {
        DistEntry entry;
        float beatA = BeatA(i);
        for (int j = 0; j < mDists.Height(); j++) {
            mDists(i, j) = sLargeFloat;
            float beatB = BeatB(j);
            if (mBeatAlign == 0 || BeatAligned(i, j)) {
                if (restricts) {
                    aBeatVar = beatA;
                    bBeatVar = beatB;
                    aStartVar = beatA - mClipA->StartBeat();
                    aEndVar = mClipA->EndBeat() - beatA;
                    bStartVar = beatB - mClipB->StartBeat();
                    bEndVar = mClipB->EndBeat() - beatB;
                    aMiddleVar = beatA - aInterp;
                    bMiddleVar = beatB - bInterp;
                    deltaVar = beatA - beatB;
                    if (restricts->Evaluate(1).Int() == 0) {
                        continue;
                    }
                }
                DistEntry &curDistEntry = distEntries[j];
                GenerateDistEntry(meshes, curDistEntry, BeatB(j), mClipB, bones);
                GenerateDistEntry(meshes, entry, BeatA(i), mClipA, bones);
                if (maxFacing > 0) {
                    // ...
                }
                if (weights.empty()) {
                    FindWeights(bones, weights, mWeightData);
                }
                float f27 = 0;
                for (int k = 0; k < entry.bones.size(); k++) {
                    f27 += DistanceSquared(entry.bones[k], curDistEntry.bones[k])
                        * weights[k % weights.size()];
                }
                float sqrted = std::sqrt((float)entry.bones.size());
                MaxEq(mWorstErr, sqrted);
                mDists(i, j) = sqrted;
            }
        }
    }
    aBeatVar = aBeatVal;
    bBeatVar = bBeatVal;
    aStartVar = aStartVal;
    aEndVar = aEndVal;
    bStartVar = bStartVal;
    bEndVar = bEndVal;
    aMiddleVar = aMiddleVal;
    bMiddleVar = bMiddleVal;
    deltaVar = deltaVal;
}
