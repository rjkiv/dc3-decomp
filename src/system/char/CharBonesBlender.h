#pragma once
#include "char/CharBones.h"
#include "char/CharPollable.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

/** "Blends itself into another CharBones, clearing self each frame" */
class CharBonesBlender : public CharPollable, public CharBonesAlloc {
public:
    virtual ~CharBonesBlender() {}
    OBJ_CLASSNAME(CharBonesBlender);
    OBJ_SET_TYPE(CharBonesBlender);
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();
    virtual void Enter();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x14);
    NEW_OBJ(CharBonesBlender);

    void SetDest(CharBonesObject *);
    void SetClipType(Symbol);

protected:
    CharBonesBlender();
    virtual void ReallocateInternal();

    /** "CharBones to blend into" */
    ObjPtr<CharBonesObject> mDest; // 0x60
    /** "What type of clip we can accommodate" */
    Symbol mClipType; // 0x74
};
