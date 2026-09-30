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
    void Set(float, float, const Transform &, const Transform &);

    float mMinX; // 0x0
    float mMaxX; // 0x4
    Vector3 mOffset; // 0x8
};

// size 0xe4
struct SongCollisionOutput {};

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

    void Update(MoveDir *);
    bool Equals(SongCollision *);
    bool IsCollision(
        int,
        int,
        const Difficulty *const,
        const Transform *const,
        std::vector<SongCollisionOutput> *
    ) const;

    static void Init();
    static void GatherUsefulBones(std::vector<RndTransformable *> &, HamCharacter *);

private:
    static float sCollisionTolerance;

    const BeatCollisionData *BeatData(int, Difficulty) const;
    void CheckCollision(
        int, const Difficulty *const, const Transform *const, SongCollisionOutput &
    ) const;

protected:
    SongCollision();

    // indexed by difficulty, then beat
    // so mData[kDifficultyExpert][4] is
    // the BeatCollisionData for expert difficulty at beat 4
    std::vector<BeatCollisionData> mData[kNumDifficulties]; // 0x2c
};
