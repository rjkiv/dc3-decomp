#include "char/CharHair.h"
#include "char/Character.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"
#include "world/Dir.h"

CharHair *gHair;
CharHair::Strand *gStrand;

CharHair::CharHair()
    : mStiffness(0.04), mTorsion(0.1), mInertia(0.7), mGravity(1), mWeight(0.5),
      mFriction(0.3), mWind(1), mFlat(1), mMinSlack(0), mMaxSlack(0), mStrands(this),
      mReset(1), mSimulate(1), mUsePostProc(1), mWindObj(this), mCollides(this),
      mManagedHookup(0) {}

CharHair::~CharHair() {}

BEGIN_HANDLERS(CharHair)
    HANDLE_ACTION(reset, mReset = _msg->Int(2))
    HANDLE_ACTION(hookup, Hookup())
    HANDLE_ACTION(set_cloth, SetCloth(_msg->Int(2)))
    HANDLE_ACTION(freeze_pose, FreezePose())
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharHair::Point)
    SYNC_PROP(bone, o.bone)
    SYNC_PROP(length, o.length)
    SYNC_PROP(collides, o.collides)
    SYNC_PROP(radius, o.radius)
    SYNC_PROP(outer_radius, o.outerRadius)
    SYNC_PROP(side_length, o.sideLength)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(CharHair::Strand)
    gStrand = &o;
    SYNC_PROP_SET(root, o.mRoot.Ptr(), o.SetRoot(_val.Obj<RndTransformable>()))
    SYNC_PROP_SET(angle, o.mAngle, o.SetAngle(_val.Float()))
    SYNC_PROP(points, o.mPoints)
    SYNC_PROP(hookup_flags, o.mHookupFlags)
    SYNC_PROP(show_spheres, o.mShowSpheres)
    SYNC_PROP(show_collide, o.mShowCollide)
    SYNC_PROP(show_pose, o.mShowPose)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(CharHair)
    gHair = this;
    SYNC_PROP(stiffness, mStiffness)
    SYNC_PROP(torsion, mTorsion)
    SYNC_PROP(inertia, mInertia)
    SYNC_PROP(gravity, mGravity)
    SYNC_PROP(weight, mWeight)
    SYNC_PROP(friction, mFriction)
    SYNC_PROP(wind_obj, mWindObj)
    SYNC_PROP(wind, mWind)
    SYNC_PROP(flat, mFlat)
    SYNC_PROP(strands, mStrands)
    SYNC_PROP(simulate, mSimulate)
    SYNC_PROP(min_slack, mMinSlack)
    SYNC_PROP(max_slack, mMaxSlack)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void operator<<(BinStream &bs, const CharHair::Point &p) {
    bs << p.pos;
    bs << p.bone;
    bs << p.length;
    bs << p.radius;
    bs << p.outerRadius;
    bs << p.sideLength;
    bs << p.unk78;
}

BEGIN_SAVES(CharHair)
    SAVE_REVS(0xD, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mStiffness;
    bs << mTorsion;
    bs << mInertia;
    bs << mGravity;
    bs << mWeight;
    bs << mFriction;
    bs << mMinSlack;
    bs << mMaxSlack;
    bs << mStrands;
    bs << mSimulate;
    bs << mWindObj;
    bs << mWind;
    bs << mFlat;
END_SAVES

void CharHair::Strand::Save(BinStream &bs) const {
    bs << mRoot;
    bs << mAngle;
    bs << mPoints;
    bs << mBaseMat;
    bs << mRootMat;
    bs << mHookupFlags;
}

BEGIN_COPYS(CharHair)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharHair)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mStiffness)
        COPY_MEMBER(mInertia)
        COPY_MEMBER(mGravity)
        COPY_MEMBER(mWeight)
        COPY_MEMBER(mFriction)
        COPY_MEMBER(mTorsion)
        COPY_MEMBER(mStrands)
        COPY_MEMBER(mSimulate)
        COPY_MEMBER(mMinSlack)
        COPY_MEMBER(mMaxSlack)
        COPY_MEMBER(mWindObj)
        COPY_MEMBER(mWind)
        COPY_MEMBER(mFlat)
    END_COPYING_MEMBERS
END_COPYS

void CharHair::SetName(const char *name, ObjectDir *dir) {
    Hmx::Object::SetName(name, dir);
    mUsePostProc = dynamic_cast<Character *>(dir) || dynamic_cast<WorldDir *>(dir);
}
