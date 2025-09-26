#pragma once
#include "char/CharPollable.h"
#include "rndobj/Highlight.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Offsets a dest bone." */
class CharBoneOffset : public CharPollable, public RndHighlightable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(CharBoneOffset);
    OBJ_SET_TYPE(CharBoneOffset);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // CharPollable
    virtual void Poll();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);
    // RndHighlightable
    virtual void Highlight() {}

    OBJ_MEM_OVERLOAD(0x16)
    NEW_OBJ(CharBoneOffset)

protected:
    CharBoneOffset();

    /** "The bone to offset" */
    ObjPtr<RndTransformable> mDest; // 0x10
    /** "the offset" */
    Vector3 mOffset; // 0x24
};
