#pragma once
#include "CharEyeDartRuleset.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "An interest object for a character to look at" */
class CharInterest : public RndTransformable {
public:
    // Hmx::Object
    virtual ~CharInterest() {}
    OBJ_CLASSNAME(CharInterest);
    OBJ_SET_TYPE(CharInterest);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void Highlight();

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(CharInterest)

    bool IsWithinViewCone(Vector3 const &, Vector3 const &);
    bool IsMatchingFilterFlags(int);
    float
    ComputeScore(const Vector3 &, const Vector3 &, const Vector3 &, float, int, bool);

protected:
    CharInterest();

    void SyncMaxViewAngle();

    /** "In degrees, the maximum view cone angle for this object to be 'seen'".
        Ranges from 0 to 90. */
    float mMaxViewAngle; // 0xc0
    /** "An extra weight applied during scoring of this interest -
        use this to make it more or less important overall". Ranges from 0 to 5. */
    float mPriority; // 0xc4
    /** "The minimum time you have to look at this object when its selected".
        Ranges from 0 to 100. */
    float mMinLookTime; // 0xc8
    /** "The maximum allowable time to look at this object". Ranges from 0 to 100. */
    float mMaxLookTime; // 0xcc
    /** "In secs, how long until this object can be looked at again".
        Ranges from 0 to 100. */
    float mRefractoryPeriod; // 0xd0
    /** "if set, this dart ruleset will override the default one
        when looking at this interest object" */
    ObjPtr<CharEyeDartRuleset> mDartRulesetOverride; // 0xd4
    int mCategoryFlags; // 0xe8
    /** "if true, we will override the minimum distance this target can be
        from the eyes using the value below" */
    bool mOverridesMinTargetDist; // 0xec
    /** "the minimum distance, in inches, that this interest can be from the eyes.
        only applied if overrides_min_target_dist is true..." */
    float mMinTargetDistOverride; // 0xf0
    float mMaxViewAngleCos; // 0xf4
};
