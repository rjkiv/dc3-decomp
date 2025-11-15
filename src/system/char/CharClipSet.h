#pragma once

#include "char/CharClip.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/MemMgr.h"

struct ObjNameSort {
    bool operator()(Hmx::Object *c1, Hmx::Object *c2) const {
        return strcmp(c1->Name(), c2->Name()) < 0;
    }
};

class CharClipSet : public ObjectDir, public RndDrawable, public RndAnimatable {
public:
    // Hmx::Object
    virtual ~CharClipSet();
    OBJ_CLASSNAME(CharClipSet)
    OBJ_SET_TYPE(CharClipSet)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreSave(BinStream &);
    virtual void PostSave(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);

    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float StartFrame();
    virtual float EndFrame();

    // RndDrawable
    virtual void ListDrawChildren(std::list<class RndDrawable *> &);

    // ObjectDir
    virtual void ResetEditorState();
    virtual void SetBpm(int);

    MEM_OVERLOAD(CharClipSet, 0x123); // change later
    NEW_OBJ(CharClipSet)

    void ResetPreviewState();
    void SortGroups();
    void LoadCharacter();

    FilePath mCharFilePath; // 0xec
    ObjPtr<RndDir> mPreviewChar; // 0xf4
    ObjPtr<CharClip> mPreviewClip; // 0x108
    int mFilterFlags; // 0x11c
    int mBpm; // 0x120
    bool mPreviewWalk; // 0x124
    ObjPtr<CharClip> mStillClip; // 0x128

protected:
    CharClipSet();
    void RecenterAll();
    DataNode OnListClips(DataArray *);
};
