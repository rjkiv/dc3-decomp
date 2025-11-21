#include "hamobj/SongCollision.h"
#include "SongCollision.h"
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Object.h"
#include "utl/Std.h"

std::vector<const char *> sCollisionUsefulBoneNames;

SongCollision::SongCollision() {}

BEGIN_PROPSYNCS(SongCollision)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

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
