#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "math/Geo.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Makes a source point at dest by rotating a pivot, points along Y axis of source" */
class CharLookAt : public RndHighlightable, public CharWeightable, public CharPollable {
public:
    // Hmx::Object
    virtual ~CharLookAt();
    OBJ_CLASSNAME(CharLookAt);
    OBJ_SET_TYPE(CharLookAt);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(CharLookAt)

    void SetMinYaw(float);
    void SetMaxYaw(float);
    void SetMinPitch(float);
    void SetMaxPitch(float);

protected:
    CharLookAt();

    void SyncLimits();

    /** "If non null, the bone which looks at along its Y axis,
        otherwise equal to the pivot" */
    ObjPtr<RndTransformable> mSource; // 0x30
    /** "The thing that pivots" */
    ObjPtr<RndTransformable> mPivot; // 0x44
    /** "The thing to look at" */
    ObjPtr<RndTransformable> mTarget; // 0x58
    /** "Seconds of lag when moving the source" */
    float mHalfTime; // 0x6c
    /** "Degrees of min allowable yaw, looking left". Ranges from -80 to 80. */
    float mMinYaw; // 0x70
    /** "Degrees of max allowable yaw, looking right". Ranges from -80 to 80. */
    float mMaxYaw; // 0x74
    /** "Degrees of min allowable pitch, looking down". Ranges from -80 to 80. */
    float mMinPitch; // 0x78
    /** "Degrees of max allowable pitch, looking up". Ranges from -80 to 80. */
    float mMaxPitch; // 0x7c
    /** "Degrees of yaw to start auto-weight, -1 means no auto-weight" */
    float mMinWeightYaw; // 0x80
    /** "Degrees of yaw to stop auto-weight, must be greater than mMinWeightYaw" */
    float mMaxWeightYaw; // 0x84
    /** "Max speed in weight/sec that the auto-weight can change" */
    float mWeightYawSpeed; // 0x88
    Vector3 unk8c; // 0x8c
    float unk9c; // 0x9c
    /** "radius in degrees of filtered source motion that's allowed through" */
    float mSourceRadius; // 0xa0
    Vector3 unka4; // 0xa4
    Box unkb4; // 0xb4
    /** "Graphically show the extreme ranges of motion" */
    bool mShowRange; // 0xd4
    /** "Graphically show range of motion with user specified values" */
    bool mTestRange; // 0xd5
    /** "if test_range is on, adjusts current pitch". Ranges from 0 to 1. */
    float mTestRangePitch; // 0xd8
    /** "if test_range is on, adjusts current yaw". Ranges from 0 to 1. */
    float mTestRangeYaw; // 0xdc
    /** "If true allows rolling, if false,
        keeps the local pivot z axis down to prevent rolling.
        Eyeballs can't roll, for instance, but heads can." */
    bool mAllowRoll; // 0xe0
    bool unke1;
    /** "If enabled, high frequency noise is added to pitch and/or yaw each frame" */
    bool mEnableJitter; // 0xe2
    /** "if enable_jitter is on, random noise from
        [-yaw_jitter_limit, yaw_jitter_limit] (in degrees)
        is applied to the yaw of the lookat". Ranges from 0 to 10. */
    float mYawJitterLimit; // 0xe4
    /** "if enable_jitter is on, random noise from
        [-pitch_jitter_limit, pitch_jitter_limit] (in degrees)
        is applied to the pitch of the lookat". Ranges from 0 to 10. */
    float mPitchJitterLimit; // 0xe8
};
