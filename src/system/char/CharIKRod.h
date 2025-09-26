#pragma once
#include "char/CharPollable.h"
#include "math/Mtx.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "<p>Rigs a bone between two other bones and sets the
    orientation from that.</p>

    <p>When you set up all the bone pointers, the rig xfm will be
    computed, an inverse from that to the dst bone will be computed,
    and everything will come from that. So the dst bone will maintain
    the exact same position in that pose. That makes it easy to author
    the bones.</p>" */
class CharIKRod : public CharPollable {
public:
    // Hmx::Object
    virtual ~CharIKRod();
    OBJ_CLASSNAME(CharIKRod);
    OBJ_SET_TYPE(CharIKRod);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x1C)
    NEW_OBJ(CharIKRod)

    void SyncBones();

protected:
    CharIKRod();

    bool ComputeRod(Transform &);

    /** "Left end of the rod" */
    ObjPtr<RndTransformable> mLeftEnd; // 0x8
    /** "Right end of the rod" */
    ObjPtr<RndTransformable> mRightEnd; // 0x1c
    /** "Fraction of the way dest is from left (0) to right(1)" */
    float mDestPos; // 0x30
    /** "Take the z axis from this bone rather than from rod end delta" */
    ObjPtr<RndTransformable> mSideAxis; // 0x34
    /** "Force the dest to be vertically upright" */
    bool mVertical; // 0x48
    /** "The bone to set" */
    ObjPtr<RndTransformable> mDest; // 0x4c
    Transform mXfm; // 0x60
};
