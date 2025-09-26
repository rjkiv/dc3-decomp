#pragma once
#include "math/Easing.h"
#include "obj/Data.h"

#include "obj/Object.h"
#include "obj/Task.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"
#include <list>

class AnimTask;

/**
 * @brief: An object that can be animated.
 * Original _objects description:
 * "Base class for animatable objects. Anim objects change
 * their state or other objects."
 */
class RndAnimatable : public virtual Hmx::Object {
public:
    enum Rate {
        k30_fps = 0,
        k480_fpb = 1,
        k30_fps_ui = 2,
        k1_fpb = 3,
        k30_fps_tutorial = 4,
        k15_fpb = 5
    };

    OBJ_CLASSNAME(Anim);
    OBJ_SET_TYPE(Anim);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual ~RndAnimatable() {}
    /** Determine whether or not this animation should loop. */
    virtual bool Loop() { return false; }
    /** Start the animation. */
    virtual void StartAnim() {}
    /** End the animation. */
    virtual void EndAnim() {}
    virtual void SetFrame(float frame, float blend);
    /** Get this animatable's first frame. */
    virtual float StartFrame() { return 0; }
    /** Get this animatable's last frame. */
    virtual float EndFrame() { return 0; }
    /** The actual target Object we want to animate. */
    virtual Hmx::Object *AnimTarget() { return this; }
    /** Set any of this Anim's keys values to any relevant anim target properties at the
     * given frame. */
    virtual void SetKey(float frame) {}
    /** Get the list of this Object's children that are animatable. */
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const {}
    virtual DataNode OnListFlowLabels(DataArray *) { return 0; }

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(RndAnimatable)

    /** Determine if this animatable has any active tasks associated with it. */
    bool IsAnimating();
    /** Kill any active tasks associated with this animatable. */
    void StopAnimation();

    Task *Animate(float, bool, float, Hmx::Object *, EaseType, float, bool);
    Task *Animate(
        float,
        bool,
        float,
        Rate,
        float,
        float,
        float,
        float,
        Symbol,
        Hmx::Object *,
        EaseType,
        float,
        bool
    );
    Task *
    Animate(float, float, TaskUnits, float, float, Hmx::Object *, EaseType, float, bool);

    TaskUnits Units() const;
    float FramesPerUnit();
    bool ConvertFrames(float &frames);

    // weak getters and setters
    Rate GetRate() { return mRate; }
    void SetRate(Rate r) { mRate = r; }
    float GetFrame() const { return mFrame; }

    static TaskUnits RateToTaskUnits(Rate);

protected:
    /** "Frame of animation". It ranges from 0 to what EndFrame() returns. */
    float mFrame; // 0x8
    /** "Rate to animate" */
    Rate mRate; // 0xc

    RndAnimatable();
    void FireFlowLabel(Symbol);
    /** Create a new AnimTask using the configuration in the supplied DataArray.
     * @param [in] arr The supplied DataArray.
     * @returns A DataNode housing the newly created task.
     * Expected DataArray contents:
     *     No specific node ordering, but the DataArray can optionally have:
     *     - data for symbols: blend, delay, units, name, wait
     *     - a DataArray for symbol range with floats at nodes 1 and 2
     *     - a DataArray for symbol loop with floats at nodes 1 and 2
     *     - a DataArray for symbol dest with a float at node 1
     *     - a DataArray for symbol period with a float at node 1
     * Example usage: {$this on_animate}
     */
    DataNode OnAnimate(DataArray *arr);
    DataNode OnConvertFrames(DataArray *);
};

/** A task meant for animating. */
class AnimTask : public Task {
public:
    AnimTask(
        RndAnimatable *,
        float,
        float,
        float,
        bool,
        float,
        Hmx::Object *,
        EaseType,
        float,
        bool
    );
    virtual ~AnimTask();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(AnimTask);
    virtual void Poll(float);

    float TimeUntilEnd();
    AnimTask *BlendTask() const { return mBlendTask; }
    RndAnimatable *Anim() const { return mAnim; }
    Hmx::Object *AnimTarget() const { return mAnimTarget; }

    POOL_OVERLOAD(AnimTask, 0x75);

    /** The animatable this task should be animating. */
    ObjOwnerPtr<RndAnimatable> mAnim; // 0x2c
    ObjPtr<Hmx::Object> unk40; // 0x40
    ObjPtr<Hmx::Object> mAnimTarget; // 0x54
    /** The anim task to blend into. */
    ObjPtr<AnimTask> mBlendTask; // 0x68
    /** Whether or not this animation should blend into another. */
    bool mBlending; // 0x7c
    /** The time it takes to blend into mBlendTask. */
    float mBlendTime; // 0x80
    float mBlendPeriod; // 0x84
    /** Start animation frame. */
    float mMin; // 0x88
    /** End animation frame. */
    float mMax; // 0x8c
    /** Multiplier to speed of animation. */
    float mScale; // 0x90
    /** "Amount to offset frame for animation" */
    float mOffset; // 0x94
    /** Whether or not the animation should loop. */
    bool mLoop; // 0x98
    float unk9c;
    EaseFunc *mEaseFunc; // 0xa0
    float unka4;
    bool unka8;
    float unkac;
    bool unkb0;
};
