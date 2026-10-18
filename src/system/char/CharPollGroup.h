#pragma once
#include "char/CharPollable.h"
#include "char/CharWeightable.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

/** "Group of Charpollable, polled in the order given,
    use when the automatic CharPollable sorting is not correct or sufficient." */
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

    OBJ_MEM_OVERLOAD(0x15)
    NEW_OBJ(CharPollGroup);

    void SortPolls();

protected:
    CharPollGroup();

    /** "Ordered list of CharPollables, will be polled in this order." */
    ObjPtrList<CharPollable> mPolls; // 0x28
    /** "Explicit thing I am changed by, to force sorting, if set, ignores polls" */
    ObjPtr<CharPollable> mChangedBy; // 0x3c
    /** "Explicit thing I change, to force sorting, if set, ignores polls" */
    ObjPtr<CharPollable> mChanges; // 0x50
};
