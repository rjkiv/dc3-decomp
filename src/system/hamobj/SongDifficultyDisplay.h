#pragma once
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UIComponent.h"
#include "utl/MemMgr.h"

/** "Song Difficulty Display" */
class SongDifficultyDisplay : public UIComponent {
public:
    // Hmx::Object
    virtual ~SongDifficultyDisplay();
    OBJ_CLASSNAME(SongDifficultyDisplay);
    OBJ_SET_TYPE(SongDifficultyDisplay);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void DrawShowing();

    OBJ_MEM_OVERLOAD(0x14)
    NEW_OBJ(SongDifficultyDisplay)
    static void Init();

    void SetLevel(int);

protected:
    SongDifficultyDisplay();

    // UIComponent
    virtual void OldResourcePreload(BinStream &);

    void Update();

    ObjPtr<RndAnimatable> mAnimation; // 0x44
    ResourceDirPtr<RndDir> mResourceDir; // 0x58
};
