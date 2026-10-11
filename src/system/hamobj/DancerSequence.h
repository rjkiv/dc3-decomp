#pragma once
#include "hamobj/DancerSkeleton.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/MemMgr.h"

// size 0x2dc
struct DancerFrame {
    /** ID of the move that this frame belongs to */
    short mMoveIdx; // 0x0

    /** Index of the frame within the move - from 1 to 16 */
    short mMoveFrameIdx; // 0x2

    /** The DancerSkeleton for this frame */
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

    /** Returns the DancerFrames - belonging to a move is done by mMoveIdx within each
     */
    const std::vector<DancerFrame> &GetDancerFrames() const;

    /** Returns the DancerSkeleton for the current DancerFrame
     */
    const DancerSkeleton *CurSkeleton() const;

protected:
    DancerSequence();

    /** DancerFrames within this DancerSequence - belonging to a move is done by mMoveIdx
     * within each
     */
    std::vector<DancerFrame> mDancerFrames; // 0x10
};
