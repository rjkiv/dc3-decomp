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

/** "A <a href='#CharClip'>CharClip</a> container." */
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
    virtual void Draw();
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<class RndDrawable *> &);

    // ObjectDir
    virtual void ResetEditorState();
    virtual void SetBpm(int);

    OBJ_MEM_OVERLOAD(0x17)
    NEW_OBJ(CharClipSet)
    REGISTER_OBJ(CharClipSet)

    void ResetPreviewState();
    void SortGroups();
    void LoadCharacter();

protected:
    CharClipSet();
    void RecenterAll();
    DataNode OnListClips(DataArray *);

    /** "Preview base character to use-
        for example, char/male/male_guitar.milo for male guitarist" */
    FilePath mCharFilePath; // 0xec
    ObjPtr<RndDir> mPreviewChar; // 0xf4
    /** "Pick a clip to play" */
    ObjPtr<CharClip> mPreviewClip; // 0x108
    /** "Flags for filtering preview clip" */
    int mFilterFlags; // 0x11c
    /** "bpm for clip playing" */
    int mBpm; // 0x120
    /** "Allow preview character to move around and walk?" */
    bool mPreviewWalk; // 0x124
    /** "Set this to view drummer play anims" */
    ObjPtr<CharClip> mStillClip; // 0x128
};
