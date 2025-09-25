#pragma once
#include "HamIKSkeleton.h"
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "char/Character.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Does IK on end effectors and props" */
class HamIKEffector : public RndHighlightable,
                      public CharWeightable,
                      public CharPollable {
public:
    class Constraint {
    public:
        Constraint(Hmx::Object *owner) : mTarget(owner), mWeight(0) {}

        /** "The thing the [effector] will keep relative xfm to,
            or absolute xfm if weight <= 0." */
        ObjPtr<RndTransformable> mTarget; // 0x0
        /** "Relative weight for this constraint,
            is same as radius in feet of full effect.
            If zero or negative [effector] will be constrained
            directly to the [target]" */
        float mWeight; // 0x14
    };
    enum EffectorType {
        kEffectorTypeNone = 0,
        kEffectorTypePelvis = 1,
        kEffectorTypeAnkle = 2,
        kEffectorTypeHand = 3,
        kEffectorTypeForearm = 4,
        kEffectorTypeHead = 5
    };

    // Hmx::Object
    virtual ~HamIKEffector();
    OBJ_CLASSNAME(HamIKEffector);
    OBJ_SET_TYPE(HamIKEffector);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void SetName(const char *, ObjectDir *);
    // RndHighlightable
    virtual void Highlight() {}
    // CharPollable
    virtual void Poll();
    virtual void ListPollChildren(std::list<RndPollable *> &) const;
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x19)
    NEW_OBJ(HamIKEffector)

protected:
    HamIKEffector();

    EffectorType GetType();
    void IKElbow(const Vector3 &);
    void
    ComputeHandPullAndQuat(QuatXfm &, Transform &, const Transform &, const Vector3 &);

    /** "pointer to a HamIKSkeleton object" */
    ObjPtr<HamIKSkeleton> mSkeleton; // 0x30
    /** "The character end effector or prop to be constrained.
        When hilit, the big red circles are the "'natural"' 'positions,'
        the blue circles are the desired 'destination,'
        and the green circle is where it was 'IK\qd' to.
        Each "target" shows up as an axis display
        with the current weight shown next to 'it."' */
    ObjPtr<RndTransformable> mEffector; // 0x44
    /** "The actual trans to IK to constraints.
        Effector is still the thing that gets moved, if NULL, same as effector" */
    ObjPtr<RndTransformable> mFinger; // 0x58
    /** "for pelvis/feet specifies a ground plane height" */
    ObjPtr<RndTransformable> mGround; // 0x6c
    /** "More constraints to evaluate" */
    ObjPtr<HamIKEffector> mMore; // 0x80
    /** "Can call others to do ik on the same bone before doing shoulder fixup" */
    ObjPtr<CharPollable> mOther; // 0x94
    /** "If hand, pointer to elbow object" */
    ObjPtr<HamIKEffector> mElbow; // 0xa8
    ObjVector<Constraint> mConstraints; // 0xbc
    ObjPtr<Character> unkcc; // 0xcc
};
