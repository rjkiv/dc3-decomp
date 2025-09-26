#pragma once
#include "char/CharDriver.h"
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

/** "Sets its own weight by pushing flags through a driver to
 *  see what fraction of them it has." */
class CharWeightSetter : public CharWeightable, public CharPollable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CharWeightSetter);
    OBJ_SET_TYPE(CharWeightSetter);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharWeightable
    virtual void SetWeight(float wt);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(CharWeightSetter)

protected:
    CharWeightSetter();

    /** "If driver not set, uses this to get base weight" */
    ObjPtr<CharWeightable> mBase; // 0x28
    /** "The Driver to monitor" */
    ObjPtr<CharDriver> mDriver; // 0x3c
    /** "Weight can't be greater than this weightable" */
    ObjPtrList<CharWeightSetter> mMinWeights; // 0x50
    /** "Weight can't be less than this weightable" */
    ObjPtrList<CharWeightSetter> mMaxWeights; // 0x64
    /** "Which clip flags to look for" */
    int mFlags; // 0x78
    /** "Constant offset to apply to the weight." */
    float mOffset; // 0x7c
    /** "Constant scale to apply to the weight." */
    float mScale; // 0x80
    /** "The base weight that the real weight is derived from" */
    float mBaseWeight; // 0x84
    /** "how many beats it should take to change the weight from 0 to 1" */
    float mBeatsPerWeight; // 0x88
};
