#pragma once
#include "char/CharCollide.h"
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Pins a hand bone to another RndTransformable, bending the elbow to make it reach.
    Optionally aligns orientations and stretches" */
class CharIKHand : public RndHighlightable, public CharWeightable, public CharPollable {
public:
    struct IKTarget {
        IKTarget(Hmx::Object *);
        IKTarget(ObjPtr<RndTransformable>, float);

        /** "Where to move the hand to" */
        ObjPtr<RndTransformable> mTarget; // 0x0
        /** "Distance along the negative z axis of the transform to snap to" */
        float mExtent; // 0x14
    };
    // Hmx::Object
    virtual ~CharIKHand();
    OBJ_CLASSNAME(CharIKHand);
    OBJ_SET_TYPE(CharIKHand);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x1A)
    NEW_OBJ(CharIKHand)

    void SetHand(RndTransformable *);
    void MeasureLengths();

protected:
    CharIKHand();
    void PullShoulder(Vector3 &, Transform const &, Vector3 const &, float);
    void IKElbow(RndTransformable *, RndTransformable *);

    /** "The hand to be moved, must be child of elbow" */
    ObjPtr<RndTransformable> mHand; // 0x30
    /** "If non null, will be the thing that actually hits the target,
        the hand will be moved into such a location as to make it hit.
        You probably always want to turn on orientation in this case, as otherwise,
        the hand will be in a somewhat random orientation,
        which will probably mean that the finger will miss the mark." */
    ObjPtr<RndTransformable> mFinger; // 0x44
    /** "Targets for the hand" */
    ObjVector<IKTarget> mTargets; // 0x58
    /** "Orient the hand to the dest" */
    bool mOrientation; // 0x68
    /** "Stretch the hand to the dest" */
    bool mStretch; // 0x69
    /** "Recalculate bone length every frame, needed for bones which scale" */
    bool mScalable; // 0x6a
    /** "Moves the elbow and shoulder to position the hand,
        if false, just teleports the hand" */
    bool mMoveElbow; // 0x6b
    /** "Range to swing the elbow in radians to hit target, better looking suggest .7" */
    float mElbowSwing; // 0x6c
    /** "Turn this on to do IK calcs even if weight is 0" */
    bool mAlwaysIKElbow; // 0x70
    /** "Are we allowed to pull the shoulder to reach goal,
        or do we lock the elbow when goal is too far?" */
    bool mPullShoulder; // 0x71
    bool mHandChanged; // 0x72
    float unk74;
    float unk78;
    float unk7c;
    float unk80;
    float unk84;
    float unk88;
    float unk8c;
    /** "Constrain the wrist rotation to be believable" */
    bool mConstraintWrist; // 0x90
    /** "Constrain wrist rotation to this angle (in radians)" */
    float mWristRadians; // 0x94
    /** "Collision sphere that elbow won't enter." */
    ObjPtr<CharCollide> mElbowCollide; // 0x98
    /** "Choose the clockwise solution for the collision detection" */
    bool mClockwise; // 0xac
};

BinStream &operator>>(BinStream &, CharIKHand::IKTarget &);
