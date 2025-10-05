#pragma once
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Object for setting up lazy transform parenting" */
class TransConstraint : public RndHighlightable, public RndPollable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(TransConstraint);
    OBJ_SET_TYPE(TransConstraint);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    OBJ_MEM_OVERLOAD(0x12)
    NEW_OBJ(TransConstraint)
    void SnapToParent();

private:
    void SetScaleVectorOnTransform(RndTransformable *, Vector3 &);

protected:
    TransConstraint();

    /** "The trans parent" */
    ObjPtr<RndTransformable> mParent; // 0x10
    /** "The trans child" */
    ObjPtr<RndTransformable> mChild; // 0x24
    /** "Dimensions of cube in which child transform is not updated" */
    Vector3 mStaticCube; // 0x38
    /** "Track x/y/z-axis to parent?" */
    bool mTracks[3]; // 0x48
    /** "Speed of tracking in inches/second". Ranges from 1 to 10000. */
    float mSpeed; // 0x4c
    /** "Whether or not to interpolate scale along with position" */
    bool mAffectScale; // 0x50
    /** "Whether or not to use UI timing.  Default is to use game time." */
    bool mUseUITime; // 0x51
    bool unk52; // 0x52
};
