#include "hamobj/SongCollision.h"
#include "SongCollision.h"
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Object.h"
#include "utl/Std.h"

std::vector<const char *> sCollisionUsefulBoneNames;

SongCollision::SongCollision() {}

BEGIN_HANDLERS(SongCollision)
    HANDLE_ACTION(update, Update(_msg->Obj<MoveDir>(2)))
    HANDLE_ACTION(print, Print())
    HANDLE_EXPR(equals, Equals(_msg->Obj<SongCollision>(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(SongCollision)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_COPYS(SongCollision)
COPY_SUPERCLASS(Hmx::Object)
CREATE_COPY(SongCollision)
BEGIN_COPYING_MEMBERS
//COPY_MEMBER_FROM(c, mData)
END_COPYING_MEMBERS
END_COPYS

BEGIN_SAVES(SongCollision)
SAVE_REVS(2, 1)
SAVE_SUPERCLASS(Hmx::Object)
END_SAVES

void SongCollision::Print() {
    int maxDatas = 0;
    for (int i = 0; i < kNumDifficulties; i++) {
        maxDatas = Max<int>(maxDatas, mData[i].size());
    }
    String str;
    str = "Beat\tData";
    for (Difficulty d = EasiestDifficulty(); d < kNumDifficulties;
         d = DifficultyOneHarder(d)) {
        str += MakeString("\t%s", DifficultyToSym(d).Str());
    }
    MILO_LOG("%s\n", str.c_str());
    // more...
}

void SongCollision::Init() {
    REGISTER_OBJ_FACTORY(SongCollision);
    DataArray *tolerance = DataGetMacro("SONG_COLLISION_TOLERANCE");
    if (tolerance) {
        sCollisionTolerance = tolerance->Float(0);
    }
    sCollisionUsefulBoneNames.clear();
    DataArray *bones = DataGetMacro("SONG_COLLISION_BONES");
    if (bones) {
        for (int i = 0; i < bones->Size(); i++) {
            sCollisionUsefulBoneNames.push_back(bones->Str(i));
        }
    }
}

const BeatCollisionData *SongCollision::BeatData(int beat, Difficulty diff) const {
    MILO_ASSERT_RANGE(diff, 0, kNumDifficulties, 0xe8);
    MILO_ASSERT(beat >= 0, 0xeb);
    if (beat < mData[diff].size()) {
        return &mData[diff][beat];
    }
    return nullptr;
}

void BeatCollisionData::Set(float minX, float minY, const Transform &start_xfm, const Transform &end_xfm) {
    using namespace Hmx;
    MILO_ASSERT(start_xfm.m == Matrix3::GetIdentity(), 0x62);
    MILO_ASSERT(end_xfm.m == Matrix3::GetIdentity(), 0x63);
    mMinX = minX;
    mMaxX = minY;
    float startXX = start_xfm.m.x.x;
    float endXX = end_xfm.m.x.x;
    float startXY = start_xfm.m.x.y;
    float endXY = end_xfm.m.x.y;
    float startXZ = start_xfm.m.x.z;
    float endXZ = end_xfm.m.x.z;
    mOffset.x = startXX - endXX;
    mOffset.y = startXY - endXY;
    mOffset.z = startXZ - endXZ;
}