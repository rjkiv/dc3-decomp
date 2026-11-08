#pragma once
#include "char/CharClip.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"

struct Alphabetically {
    bool operator()(const CharClip *c1, const CharClip *c2) const {
        return strcmp(c1->Name(), c2->Name()) < 0;
    }
};

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
    REGISTER_OBJ(CharClipGroup)

    CharClip *GetClip(int flags);
    //  public: CharClip* GetClip();
    //  public: void MakeMRU(CharClip*);
    //  public: void MakeMRU(int32_t);
    //  public: int32_t Size() const;
    void DeleteRemaining(int remaining);
    void AddClip(CharClip *c);
    //  public: void RemoveClip(CharClip*);
    //  public: bool HasClip(int32_t) const;
    bool HasClip(CharClip *c) const;
    CharClip *FindClip(const char *name) const;
    void Sort();
    void SetClipFlags(int flags);

    ObjPtrVec<CharClip> &Clips() { return mClips; }
    int GetFlags() const { return mFlags; }
    void SetFlags(int flags) { mFlags = flags; }

    //  public: bool FlagUnique(CharClip*, int32_t);
    //  public: int32_t NumFlagDuplicates(CharClip*, int32_t);

protected:
    CharClipGroup();

    int PosMod(int) const;
    int NegMod(int) const;
    int QueueRandom(int start, int end) const;

    /** "LRU list of clips belonging to this group" */
    ObjPtrVec<CharClip> mClips; // 0x4
    int mWhich; // 0x20
    int mQueueStart; // 0x24
    int mFlags; // 0x28
};
