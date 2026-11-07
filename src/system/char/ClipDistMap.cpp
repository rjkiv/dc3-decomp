#include "char/ClipDistMap.h"
#include "char/CharBoneDir.h"
#include "char/CharBonesMeshes.h"
#include "char/CharClip.h"
#include "char/CharClipDriver.h"
#include "char/CharUtl.h"
#include "math/Geo.h"
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

static const float kNoData = kHugeFloat;

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
    CharClip *a,
    CharClip *b,
    float beatAlign,
    float blendWidth,
    int numSamples,
    const DataArray *weightData
)
    : mClipA(a), mClipB(b), mWeightData(weightData), mSamplesPerBeat(8),
      mLastMinErr(kNoData), mBeatAlign(beatAlign), mBeatAlignOffset(0),
      mBlendWidth(blendWidth), mNumSamples(numSamples),
      mDists(CalcWidth(), CalcHeight()) {
    mBeatAlignPeriod = mBeatAlign * (float)mSamplesPerBeat + 0.5;
    if (mBeatAlignPeriod != 0) {
        int samplesA = IndexA(0);
        int samplesB = IndexB(0);
        mBeatAlignOffset = Mod(samplesA - samplesB, mBeatAlignPeriod);
    }
}

bool ClipDistMap::BeatAligned(int i, int j) {
    return Mod(i - j, mBeatAlignPeriod) == mBeatAlignOffset;
}

bool ClipDistMap::FindBestNode(float minErr, float a, float b, ClipDistMap::Node &n) {
    if (a >= b) {
        return false;
    } else {
        n.err = minErr;
        int i = Max<int>(IndexA(a), 0);
        int distsWidth = Min<int>(IndexA(b), mDists.Width());
        for (; i < distsWidth; i++) {
            float a = BeatA(i);
            for (int j = mDists.Height() - 1; j >= 0; j--) {
                if (MinEq(n.err, mDists(i, j))) {
                    n.a = a;
                    n.b = BeatB(j);
                }
            }
        }
        return n.err < minErr;
    }
}

void ClipDistMap::FindNodes(float minErr, float maxDist, float endDist) {
    mNodes.clear();
    mLastMinErr = minErr;

    float f7 = maxDist * 0.45f;
    if (maxDist == 0) {
        f7 = kNoData;
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
    mAEnd = BeatA(i1 - 1);
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
    rect.x = IndexA(f3) * 2.0f + (x - 1.0f);
    rect.y = ((float)(mDists.Height() - 1) - IndexB(f4)) * 2.0f + y + 1.0f;
    TheRnd.DrawRect(rect, color, nullptr, nullptr, nullptr);
}

bool ClipDistMap::LocalMin(int i, int j) {
    float data = mDists(i, j);
    if (data == kNoData) {
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
                        if (curData != kNoData && curData < data) {
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

float ClipDistMap::BeatA(int i) { return mAStart + (float)i / (float)mSamplesPerBeat; }
float ClipDistMap::BeatB(int i) { return mBStart + (float)i / (float)mSamplesPerBeat; }
float ClipDistMap::IndexA(float i) { return (i - mAStart) * (float)mSamplesPerBeat; }
float ClipDistMap::IndexB(float i) { return (i - mBStart) * (float)mSamplesPerBeat; }

void ClipDistMap::FindDists(float maxFacing, DataArray *restricts) {
    CharBoneDir *rsrcDir = mClipA->GetResource();
    CharUtlBoneSaver saver(rsrcDir);
    CharBonesMeshes meshes;
    meshes.SetName("tmp_bones", rsrcDir);
    rsrcDir->StuffBones(meshes, mClipA->GetContext());
    std::vector<RndTransformable *> bones;
    for (ObjDirItr<RndTransformable> it(rsrcDir, true); it != nullptr; ++it) {
        if (strnicmp(it->Name(), "bone_", 4) == 0) {
            bones.push_back(it);
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
        float beatA = BeatA(i);
        DistEntry entry;
        for (int j = 0; j < mDists.Height(); j++) {
            mDists(i, j) = kNoData;
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
                    float f38 = entry.facing[0];
                    float f40 = 1.0f / 3.0f;
                    for (int k = 0; k < 3; k++) {
                        float f30 = LimitAng(entry.facing[k + 1] - entry.facing[k]);
                        float f41 =
                            LimitAng(curDistEntry.facing[k + 1] - curDistEntry.facing[k]);
                        f38 += (1 - f40) * f30 + f40 * f41;
                        f40 += 1.0f / 3.0f;
                    }
                    if (fabsf(LimitAng(f38 - curDistEntry.facing[3])) > maxFacing) {
                        mDists(i, j) = kNoData;
                        continue;
                    }
                }
                if (weights.empty()) {
                    FindWeights(bones, weights, mWeightData);
                }
                float f27 = 0;
                for (int k = 0; k < entry.bones.size(); k++) {
                    float wt = weights[k % weights.size()];
                    f27 += DistanceSquared(entry.bones[k], curDistEntry.bones[k]) * wt;
                }
                float sqrted = sqrtf(f27 / (float)entry.bones.size());
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

void ClipDistMap::Draw(float x, float y, CharDriver *d) {
    Hmx::Rect r(x - 1, y - 1, 1, ((float)mDists.Height() + 1) * 2);
    TheRnd.DrawRect(r, Hmx::Color(1, 1, 1), nullptr, nullptr, nullptr);

    r.x = (float)(mDists.Width() * 2) + x;
    TheRnd.DrawRect(r, Hmx::Color(1, 1, 1), nullptr, nullptr, nullptr);

    r.x = x - 1;
    r.h = 1;
    r.w = ((float)mDists.Width() + 1) * 2;
    TheRnd.DrawRect(r, Hmx::Color(1, 1, 1), nullptr, nullptr, nullptr);

    r.y = (float)(mDists.Height() * 2) + y;
    TheRnd.DrawRect(r, Hmx::Color(1, 1, 1), nullptr, nullptr, nullptr);

    for (float fIt = ceilf(BeatA(0)); fIt < BeatA(mDists.Width()); fIt += 1) {
        r.x = IndexA(fIt) * 2 + x;
        r.w = 2;
        TheRnd.DrawRect(r, Hmx::Color(0, 0, 0), nullptr, nullptr, nullptr);
    }

    for (float fIt = ceilf(BeatB(0)); fIt < BeatB(mDists.Height()); fIt += 1) {
        r.x = x - 1;
        r.y = ((float)(mDists.Height() - 1) - IndexB(fIt)) * 2 + y;
        r.h = 2;
        r.w = 1;
        TheRnd.DrawRect(r, Hmx::Color(0, 0, 0), nullptr, nullptr, nullptr);
    }

    Hmx::Rect wr(0, 0, 2, 2);
    for (int i = 0; i < mDists.Width(); i++) {
        wr.x = (float)(i * 2) + x;
        for (int j = 0; j < mDists.Height(); j++) {
            float data = mDists(i, j);
            if (data != kNoData) {
                float err = (mWorstErr - data) / mWorstErr;
                Hmx::Color c(err, err, err);
                wr.y = (float)((mDists.Height() - j - 1) * 2) + y;
                if (data > mLastMinErr) {
                    c.Set((err + 1) / 2, 0, 0);
                }
                TheRnd.DrawRect(wr, c, nullptr, nullptr, nullptr);
            }
        }
    }

    for (int i = 0; i < mDists.Width(); i++) {
        wr.x = (float)(i * 2) + x;
        for (int j = 0; j < mDists.Height(); j++) {
            wr.y = (float)((mDists.Height() - j - 1) * 2) + y;
            if (LocalMin(i, j)) {
                TheRnd.DrawRect(wr, Hmx::Color(1, 1, 0), nullptr, nullptr, nullptr);
            }
        }
    }

    for (int i = 0; i < mNodes.size(); i++) {
        Node &n = mNodes[i];
        DrawDot(x + 1, y - 1, n.a, n.b, Hmx::Color(1, 0, 0));
    }

    if (d && d->First()) {
        float startBeatA = mClipA->StartBeat();
        for (CharClipDriver *it = d->First(); it != nullptr; it = it->Next()) {
            if (it->Clip() == mClipA) {
                startBeatA = it->mBeat;
            }
        }
        float startBeatB = mClipB->StartBeat();
        for (CharClipDriver *it = d->First(); it != nullptr; it = it->Next()) {
            if (it->Clip() == mClipB) {
                startBeatB = it->mBeat;
                break;
            }
        }
        if (mClipA == mClipB && startBeatA == startBeatB) {
            startBeatB = mBStart;
        }
        DrawDot(x, y, startBeatA, startBeatB, Hmx::Color(0, 1, 1));
    }
}
