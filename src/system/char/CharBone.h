#pragma once
#include "char/CharBones.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "A Character system bone, conveys degrees of freedom to animate in a CharClip, lives
 * in CharBoneDir" */
class CharBone : public Hmx::Object {
public:
    struct WeightContext {
        WeightContext() : context(0), weight(0) {}
        WeightContext(int ctx, float wt) : context(ctx), weight(wt) {}

        /** "context for this bone weight" */
        int context; // 0x0
        /** "bone weight for this context" */
        float weight; // 0x4
    };
    OBJ_CLASSNAME(CharBone);
    OBJ_SET_TYPE(CharBone);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x1B);
    NEW_OBJ(CharBone);
    REGISTER_OBJ(CharBone);

    void ClearContext(int mask);
    void StuffBones(std::list<CharBones::Bone> &bones, int context) const;
    float GetWeight(int context) const;

    Symbol ChannelName(CharBones::Type t) const {
        return CharBones::ChannelName(Name(), t);
    }
    void AddWeight(int ctx, float wt) { mWeights.push_back(WeightContext(ctx, wt)); }

    int PositionContext() const { return mPositionContext; }
    void SetPositionContext(int c) { mPositionContext = c; }
    CharBones::Type Rotation() const { return mRotation; }
    void SetRotation(CharBones::Type ty) { mRotation = ty; }
    int ScaleContext() const { return mScaleContext; }
    int RotationContext() const { return mRotationContext; }
    void SetRotationContext(int c) { mRotationContext = c; }
    CharBone *GetTarget() { return mTarget; }
    void SetTarget(CharBone *b) { mTarget = b; }
    RndTransformable *Trans() { return mTrans; }
    void SetTrans(RndTransformable *t) { mTrans = t; }
    bool BakeOutAsTopLevel() { return mBakeOutAsTopLevel; }

    //  public: bool Used() const;
    //  public: bool UsedRecurse();

protected:
    CharBone();

    const WeightContext *FindWeight(int mask) const;
    WeightContext *FindWeight(int mask);

    DataNode OnGetContextFlags(DataArray *);

    /** "when to animate position" */
    int mPositionContext; // 0x2c
    /** "when to animate scale" */
    int mScaleContext; // 0x30
    /** "what axis or axes to animate". Types are: kRotNone, kRotFull, kRotX, kRotY, kRotZ
     * (???) */
    CharBones::Type mRotation; // 0x34
    /** "when to use this rotation context" */
    int mRotationContext; // 0x38
    /** "If set will exactly track this target during keyframe acquisition, used for IK
     * bones" */
    ObjPtr<CharBone> mTarget; // 0x3c
    /** "array of contexts and corresponding bone weights" */
    std::list<WeightContext> mWeights; // 0x50
    /** "Object to specify trans info" */
    ObjPtr<RndTransformable> mTrans; // 0x58
    /** "When baking out facing, use this as a top-level bone (like the pelvis)" */
    bool mBakeOutAsTopLevel; // 0x6c
};
