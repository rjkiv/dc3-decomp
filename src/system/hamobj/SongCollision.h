#pragma once
#include "MoveDir.h"
#include "hamobj/Difficulty.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

struct BeatCollisionData {
    void Set(float, float, const Transform &, const Transform &);

    float mMinX;
    float mMaxX;
    Vector3 mOffset;
};

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

    static void Init();
    void Update(MoveDir *);
    bool Equals(SongCollision *);

private:
    static float sCollisionTolerance;
    const BeatCollisionData *BeatData(int, Difficulty) const;

protected:
    SongCollision();

    std::vector<BeatCollisionData> mData[kNumDifficulties]; // 0x2c
};
