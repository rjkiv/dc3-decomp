#pragma once
#include "char/CharCollide.h"
#include "char/CharPollable.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "rndobj/Wind.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

/** "Hair physics, deals with strands of hair" */
class CharHair : public RndHighlightable, public CharPollable {
public:
    struct Point {
        Point(Hmx::Object *);

        Vector3 pos; // 0x0
        Vector3 force; // 0x10
        Vector3 lastFriction; // 0x20
        Vector3 lastZ; // 0x30
        /** "hair bone we set the transform of" */
        ObjPtr<RndTransformable> bone; // 0x40
        /** "the length of this strand bone" */
        float length; // 0x54
        /** "things to collide against" */
        ObjPtrList<CharCollide> collides; // 0x58
        /** "collision radius" */
        float radius; // 0x6c
        /** "if > radius, is the distance the hair bone should start aligning itself
            with the collision primitive, so that once touching it,
            it will be totally flattened against it." */
        float outerRadius; // 0x70
        /** "if >= 0 the base length to the side modified by min_slack and max_slack" */
        float sideLength; // 0x74
        Vector3 unk78; // 0x78
    };

    class Strand {
        friend bool
        PropSync(Strand &o, DataNode &_val, DataArray *_prop, int _i, PropOp _op);

    public:
        Strand(Hmx::Object *);
        void SetRoot(RndTransformable *);
        void SetAngle(float);
        void Save(BinStream &) const;

    private:
        /** "show the points as spheres" */
        bool mShowSpheres; // 0x0
        /** "show the collision shapes" */
        bool mShowCollide; // 0x1
        /** "Show the original pose when hilit,
            good for adjusting angle to match the pose" */
        bool mShowPose; // 0x2
        /** "The root Trans for the hair strand" */
        ObjPtr<RndTransformable> mRoot; // 0x4
        /** "Angle in degrees of starting flip, this is to counter gravity,
            because mostly models are authored under gravitational load" */
        float mAngle; // 0x18
        ObjVector<Point> mPoints; // 0x1c
        Hmx::Matrix3 mBaseMat; // 0x2c
        Hmx::Matrix3 mRootMat; // 0x5c
        int mHookupFlags; // 0x8c
    };
    // Hmx::Object
    virtual ~CharHair();
    OBJ_CLASSNAME(CharHair);
    OBJ_SET_TYPE(CharHair);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void SetName(const char *, class ObjectDir *);
    // RndHighlightable
    virtual void Highlight() {}
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x22)
    NEW_OBJ(CharHair)

protected:
    CharHair();

    void Hookup();
    void SetCloth(bool);
    void FreezePose();

    /** "stiffness of each strand, 1 means absolutely stiff, can't be bent".
        Ranges from 0 to 1. */
    float mStiffness; // 0x10
    /** "rotational stiffness of each strand, 1 means cannot be twisted".
        Ranges from 0 to 1. */
    float mTorsion; // 0x14
    /** "Inertia of the hair, one is normal, less simulates drag or other resistance".
        Ranges from 0 to 1. */
    float mInertia; // 0x18
    /** "Gravity of the hair, one is normal". Ranges from 0 to 10. */
    float mGravity; // 0x1c
    /** "Gravity of the hair, one is normal". Ranges from 0 to 1. */
    float mWeight; // 0x20
    /** "Hair friction against each other, is proportional to bending speed,
        0 means hair is a perfect spring, .1 or so is normal". Ranges from 0 to 1. */
    float mFriction; // 0x24
    /** "Wind resistance on hair, 1 is normal, used even if no wind_obj,
        proportional to surface area per point divided by mass per point."
        Ranges from 0 to 1. */
    float mWind; // 0x28
    /** "How flat the surface is, for purposes of wind resistance,
        1 means zero wind resistance perpendicular to z axis
        (the blue highlight segments)" */
    float mFlat; // 0x2c
    /** "If using sides, determines how far in it could go" */
    float mMinSlack; // 0x30
    /** "If using sides, determines how far out it could go" */
    float mMaxSlack; // 0x34
    ObjVector<Strand> mStrands; // 0x38
    int mReset; // 0x48
    /** "Simulate physics or not" */
    bool mSimulate; // 0x4c
    bool mUsePostProc; // 0x4d
    /** "wind object to use" */
    ObjPtr<RndWind> mWindObj; // 0x50
    ObjPtrList<CharCollide> mCollides; // 0x64
    bool mManagedHookup; // 0x78
};

inline BinStream &operator<<(BinStream &bs, const CharHair::Strand &s) {
    s.Save(bs);
    return bs;
}
