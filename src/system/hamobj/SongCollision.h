#pragma once
#include "MoveDir.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCharacter.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

struct BeatCollisionData {
    void
    Set(float min_x, float max_x, const Transform &start_xfm, const Transform &end_xfm);

    float mMinX; // 0x0
    float mMaxX; // 0x4
    Vector3 mOffset; // 0x8
};

// size 0xe4
struct SongCollisionOutput {
    Vector3 mMinOffsets[2]; // 0x0
    Vector3 mMaxOffsets[2]; // 0x20
    Vector3 mCollideProjections[2]; // 0x40
    Transform mPlayerXfms[2]; // 0x60
    bool mCollision; // 0xe0
};

/** "Contains data for handling potential character collisions" */
class SongCollision : public Hmx::Object {
public:
    // Hmx::Object
    OBJ_CLASSNAME(SongCollision);
    OBJ_SET_TYPE(SongCollision);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Print();

    OBJ_MEM_OVERLOAD(0x2D)
    NEW_OBJ(SongCollision)

    void Update(MoveDir *moves);
    bool Equals(SongCollision *collision);
    bool IsCollision(
        int start_beat,
        int end_beat,
        const Difficulty *const difficulties,
        const Transform *const xfms,
        std::vector<SongCollisionOutput> *outputs
    ) const;

    static void Init();
    static void
    GatherUsefulBones(std::vector<RndTransformable *> &usefulBones, HamCharacter *dancer);
    static float CollisionTolerance() { return sCollisionTolerance; }

private:
    static float sCollisionTolerance;

    const BeatCollisionData *BeatData(int beat, Difficulty diff) const;
    void CheckCollision(
        int beat,
        const Difficulty *const difficulties,
        const Transform *const xfms,
        SongCollisionOutput &output
    ) const;

protected:
    SongCollision();

    // indexed by difficulty, then beat
    // so mDiffBeatCollisions[kDifficultyExpert][4] is
    // the BeatCollisionData for expert difficulty at beat 4
    std::vector<BeatCollisionData> mDiffBeatCollisions[kNumDifficulties]; // 0x2c
};
