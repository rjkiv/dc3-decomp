#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

class CharPollGroup : public CharPollable, public CharWeightable {
public:
    // Hmx::Object
    virtual ~CharPollGroup();
    OBJ_CLASSNAME(CharPollGroup)
    OBJ_SET_TYPE(CharPollGroup)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    virtual void ListPollChildren(std::list<RndPollable *> &) const;
    virtual void PollDeps(std::list<Hmx::Object *> &, std::list<Hmx::Object *> &);

    NEW_OBJ(CharPollGroup);

    void SortPolls();

    ObjPtrList<CharPollable> mPolls; // 0x28
    ObjPtr<CharPollable> mChangedBy; // 0x3c
    ObjPtr<CharPollable> mChanges; // 0x50

protected:
    CharPollGroup();
};
