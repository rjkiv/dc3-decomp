#pragma once
#include "rndobj/Anim.h"
#include "obj/Object.h"

/**
 * @brief: A filter for an animatable object.
 * Original _objects description:
 * "An AnimFilter object modifies the playing of another animatable object"
 */
class RndAnimFilter : public RndAnimatable {
public:
    enum Type {
        kRange = 0,
        kLoop = 1,
        kShuttle = 2,
    };
    OBJ_CLASSNAME(AnimFilter);
    OBJ_SET_TYPE(AnimFilter);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual bool Loop() { return mType >= kLoop; }
    virtual void SetFrame(float, float);
    virtual float StartFrame();
    virtual float EndFrame();
    virtual Hmx::Object *AnimTarget() { return mAnim; }
    virtual void ListAnimChildren(std::list<RndAnimatable *> &) const;

    /** Set the animatable object to receive this filter. */
    void SetAnim(RndAnimatable *);

    Type GetType() { return mType; }
    RndAnimatable *Anim() { return mAnim; }
    float Start() { return mStart; }
    float End() { return mEnd; }
    float Period() { return mPeriod; }
    float FrameOffset() {
        float ret = mStart;
        if (mEnd >= mStart)
            ret = 0.0f;
        else
            ret -= mEnd;
        return mOffset + ret;
    }

protected:
    RndAnimFilter();

    /** Get this animatable's filtered scale value. */
    float Scale();
    DataNode OnSafeAnims(DataArray *);

    /** "Animatable object that receives filtered frame" */
    ObjPtr<RndAnimatable> mAnim; // 0x10
    /** "Alternative to scale, overriden period of animation" */
    float mPeriod; // 0x24
    /** "Overriden start frame of animation" */
    float mStart; // 0x28
    /** "Overriden end frame of animation" */
    float mEnd; // 0x2c
    /** "Multiplier to speed of animation" */
    float mScale; // 0x30
    /** "Amount to offset frame for animation" */
    float mOffset; // 0x34
    /** "Snap frame to nearest multiple" */
    float mSnap; // 0x38
    /** "Jitter frame randomly up to this amount" */
    float mJitter; // 0x3c
    float mJitterFrame; // 0x40
    /** "How to treat the frame outside of start and end" (range, loop or shuttle) */
    Type mType; // 0x44
};
