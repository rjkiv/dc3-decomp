#pragma once
#include "flow/FlowLabelProvider.h"
#include "flow/FlowNode.h"
#include "flow/FlowPtr.h"
#include "math/Easing.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "utl/MemMgr.h"

/** "Plays an animation" */
class FlowAnimate : public FlowNode, public FlowLabelProvider {
public:
    // Hmx::Object
    virtual ~FlowAnimate();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(FlowAnimate)
    OBJ_SET_TYPE(FlowAnimate)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // FlowNode
    virtual bool Activate();
    virtual void Deactivate(bool);
    virtual void ChildFinished(FlowNode *);
    virtual void RequestStop();
    virtual void RequestStopCancel();
    virtual void Execute(QueueState);
    virtual bool IsRunning();

    OBJ_MEM_OVERLOAD(0x1C)
    NEW_OBJ(FlowAnimate)

protected:
    FlowAnimate();

    void ResetAnim();
    void OnAnimEvent(Symbol);

    ObjOwnerPtr<AnimTask> unk5c; // 0x5c
    /** "Anim object to animate" */
    FlowPtr<RndAnimatable> mAnim; // 0x70
    /** "How should we handle stop requests?" */
    StopMode mStopMode; // 0x90
    int unk94;
    int unk98;
    /** "Blend time, does not work on Property Animations!" */
    float mBlend; // 0x9c
    /** "wait until current animation finishes before starting" */
    bool mWait; // 0xa0
    /** "delay in units before starting this animation" */
    float mDelay; // 0xa4
    /** "Enable animation filtering" */
    bool mEnable; // 0xa8
    /** "Rate to animate" */
    RndAnimatable::Rate mRate; // 0xac
    /** "Start frame of animation" */
    float mStart; // 0xb0
    /** "End frame of animation" */
    float mEnd; // 0xb4
    /** "Period of animation if non-zero" */
    float mPeriod; // 0xb8
    /** "Scale of animation" */
    float mScale; // 0xbc
    /** "How the animation is played". Possible options:
        (range "Play from [start] frame to [end] frame, then stop")
        (loop "Loop animation from [start] to [end] frame")
        (dest "Play from current frame to [end] frame")
    */
    Symbol mType; // 0xc0
    bool unkc4;
    /** "Easing to apply to animation" */
    EaseType mEase; // 0xc8
    /** "Modifier to easing equation" */
    float mEasePower; // 0xcc
    /** "Wraps animation frame values into range rather than clamping them.
    This will make the animation loop when the frame is out of range." */
    bool mWrap; // 0xd0
    /** "If true, Flow will not track/stop or otherwise affect this animation again." */
    bool mImmediateRelease; // 0xd1
};
