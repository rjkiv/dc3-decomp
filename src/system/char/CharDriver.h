#pragma once
#include "char/CharBones.h"
#include "char/CharWeightable.h"
#include "char/CharPollable.h"
#include "rndobj/Highlight.h"

class CharDriver : public RndHighlightable, public CharWeightable, public CharPollable {
public:
    // Hmx::Object
    virtual ~CharDriver();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(CharDriver);
    OBJ_SET_TYPE(CharDriver);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight();
    // CharPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    ObjectDir *ClipDir() const { return mClips; }

protected:
    CharDriver();

    /** "The CharBones object to add or blend into." */
    ObjPtr<CharBonesObject> mBones; // 0x28
    /** "pointer to clips object" */
    ObjPtr<ObjectDir> mClips; // 0x34
};
