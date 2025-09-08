#pragma once
#include "char/CharClip.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

/** "A related group of animations.  Gives you the lru one.  Usually no extension." */
class CharClipGroup : public virtual Hmx::Object {
public:
    OBJ_CLASSNAME(CharClipGroup);
    OBJ_SET_TYPE(CharClipGroup);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x14);
    NEW_OBJ(CharClipGroup)

    CharClip *GetClip(int);
    void DeleteRemaining(int);
    bool HasClip(CharClip *) const;
    void AddClip(CharClip *);
    void SetClipFlags(int);
    CharClip *FindClip(const char *) const;

protected:
    CharClipGroup();

    /** "LRU list of clips belonging to this group" */
    ObjPtrVec<CharClip> mClips; // 0x4
    int mWhich; // 0x20
    int unk24;
    int mFlags; // 0x28
};
