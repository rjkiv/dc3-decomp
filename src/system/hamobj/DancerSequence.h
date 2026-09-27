#pragma once
#include "hamobj/DancerSkeleton.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/MemMgr.h"

// size 0x2dc
struct DancerFrame {
    short mMoveIdx; // 0x0
    short mMoveFrameIdx; // 0x2
    DancerSkeleton mSkeleton; // 0x4
};

/** "Linear sequence of DancerFrame structs, animatable for preview in milo" */
class DancerSequence : public RndAnimatable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(DancerSequence);
    OBJ_SET_TYPE(DancerSequence);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void SetFrame(float frame, float blend);
    virtual float StartFrame() { return 0; }
    virtual float EndFrame();

    OBJ_MEM_OVERLOAD(0x1E);
    NEW_OBJ(DancerSequence);

    const std::vector<DancerFrame> &GetDancerFrames() const;
    const DancerSkeleton *CurSkeleton() const;

protected:
    DancerSequence();

    std::vector<DancerFrame> mDancerFrames; // 0x10
};
