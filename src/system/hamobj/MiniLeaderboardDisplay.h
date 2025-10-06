#pragma once
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UIComponent.h"
#include "utl/MemMgr.h"

/** "Mini Leaderboard Display" */
class MiniLeaderboardDisplay : public UIComponent {
public:
    // Hmx::Object
    virtual ~MiniLeaderboardDisplay();
    OBJ_CLASSNAME(MiniLeaderboardDisplay);
    OBJ_SET_TYPE(MiniLeaderboardDisplay);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    // UIComponent
    virtual void OldResourcePreload(BinStream &);

    OBJ_MEM_OVERLOAD(0x11)
    NEW_OBJ(MiniLeaderboardDisplay)
    static void Init();

protected:
    MiniLeaderboardDisplay();

    virtual void Update() {}

    ResourceDirPtr<RndDir> mResourceDir; // 0x44
};
