#pragma once
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"
#include "RhythmDetector.h"

class RhythmDetectorGroup : public RndPollable {
public:
    // Hmx::Object
    virtual ~RhythmDetectorGroup();
    OBJ_CLASSNAME(RhythmDetectorGroup);
    OBJ_SET_TYPE(RhythmDetectorGroup);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter() { RndPollable::Enter(); }
    virtual void Exit() { RndPollable::Exit(); }

    OBJ_MEM_OVERLOAD(0x13)
    NEW_OBJ(RhythmDetectorGroup)

    void SetSkeletonIndex(int);
    void RemoveDebugGraphs();
    void AddDebugGraphs();

protected:
    RhythmDetectorGroup();

    ObjPtrVec<RhythmDetector> mDetectors; //  0xC?
    float mRating; // 0x24
    float mFreshness; // 0x28
    int mSkeletonIndex; // 0x2c
    DebugGraph *mDebugGraph; // 0x30
};
