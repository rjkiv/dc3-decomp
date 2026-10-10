#include "char/ClipGraphGen.h"
#include "char/CharClip.h"
#include "char/ClipDistMap.h"
#include "obj/Data.h"
#include "obj/Object.h"

ClipGraphGenerator::ClipGraphGenerator() : mGraphData(0), mDmap(0), mClipA(0), mClipB(0) {}

ClipGraphGenerator::~ClipGraphGenerator() {}

BEGIN_HANDLERS(ClipGraphGenerator)
    HANDLE(generate_transitions, OnGenerateTransitions)
END_HANDLERS

ClipDistMap *ClipGraphGenerator::GeneratePair(
    CharClip *c1, CharClip *c2, ClipDistMap::Node *n1, ClipDistMap::Node *n2
) {
    c1->GetTransitions().RemoveClip(c2);
    mGraphData = c1->TypeDef();
    if (mGraphData) {
        if (c1->Type() == c2->Type() && ((c1->PlayFlags() & 0xF0) != 0x10)) {
            DataArray *transarr = mGraphData->FindArray("on_transition", false);
            if (transarr) {
                static DataNode &a_clip = DataVariable("a_clip");
                static DataNode &b_clip = DataVariable("b_clip");
                mDmap = nullptr;
                a_clip = c1;
                b_clip = c2;
                mClipA = c1;
                mClipB = c2;
                transarr->ExecuteScript(1, this, nullptr, 1);
                ClipDistMap *dmap = mDmap;
                mDmap = nullptr;
                if (dmap) {
                    dmap->SetNodes(n1, n2);
                }
                return dmap;
            }
        }
    }
    return nullptr;
}

DataNode ClipGraphGenerator::OnGenerateTransitions(DataArray *da) {
    MILO_ASSERT(!mDmap, 0xc6);
    MILO_ASSERT(mClipA, 0xc7);
    MILO_ASSERT(mClipB, 0xc8);
    float max_error = 1e+30f;
    da->FindData("max_error", max_error, false);
    float beat_align = 0;
    da->FindData("beat_align", beat_align, false);
    float blend_width = 1.0f;
    da->FindData("blend_width", blend_width, false);
    float max_facing = 0;
    da->FindData("max_facing", max_facing, false);
    float max_dist = 0;
    da->FindData("max_dist", max_dist, false);
    float end_dist = 0;
    da->FindData("end_dist", end_dist, false);
    DataArray *restrictArr = da->FindArray("restrict", false);

    beat_align =
        Max(beat_align,
            (float)(Min(mClipA->PlayFlags() >> 12 & 15, mClipB->PlayFlags() >> 12 & 15)));

    DataArray *boneweightarr = mGraphData->FindArray("transition_bone_weights", false);
    mDmap = new ClipDistMap(mClipA, mClipB, beat_align, blend_width, 3, boneweightarr);
    mDmap->FindDists(max_facing * DEG2RAD, restrictArr);
    mDmap->FindNodes(max_error, max_dist, end_dist);
    return 0;
}
