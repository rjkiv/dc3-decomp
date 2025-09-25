#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Rescales a local position of a bone" */
class CharIKScale : public CharWeightable, public CharPollable {
public:
    // Hmx::Object
    virtual ~CharIKScale();
    OBJ_CLASSNAME(CharIKScale);
    OBJ_SET_TYPE(CharIKScale);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x17)
    NEW_OBJ(CharIKScale)

    /** "Call after posing normally, captures data with which to auto compute scale" */
    void CaptureBefore();
    /** "Call after posing deformed, computes scale based on ratio of capture_before
     * capture to now." */
    void CaptureAfter();

protected:
    CharIKScale();

    /** "The bone to be scaled" */
    ObjPtr<RndTransformable> mDest; // 0x28
    /** "Scale to apply" */
    float mScale; // 0x3c
    /** "Apply remainder weight to these targets" */
    ObjPtrList<RndTransformable> mSecondaryTargets; // 0x40
    /** "If dest starts out at or below this height, weight will be 0" */
    float mBottomHeight; // 0x54
    /** "If dest starts out at or above this height, weight will be 1" */
    float mTopHeight; // 0x58
    /** "Automatically determine weight from top & bottom heights" */
    bool mAutoWeight; // 0x5c
};
