#include "hamobj/MiniLeaderboardDisplay.h"
#include "MiniLeaderboardDisplay.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIComponent.h"

MiniLeaderboardDisplay::MiniLeaderboardDisplay() : mResourceDir(this) {}
MiniLeaderboardDisplay::~MiniLeaderboardDisplay() {}

BEGIN_HANDLERS(MiniLeaderboardDisplay)
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_PROPSYNCS(MiniLeaderboardDisplay)
    SYNC_PROP_MODIFY(resource, mResourceDir, Update())
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(MiniLeaderboardDisplay)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(UIComponent)
    bs << mResourceDir;
END_SAVES

BEGIN_COPYS(MiniLeaderboardDisplay)
    CREATE_COPY_AS(MiniLeaderboardDisplay, p)
    MILO_ASSERT(p, 0x21);
    COPY_SUPERCLASS_FROM(UIComponent, p)
    COPY_MEMBER_FROM(p, mResourceDir)
    Update();
END_COPYS

BEGIN_LOADS(MiniLeaderboardDisplay)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void MiniLeaderboardDisplay::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    UIComponent::PreLoad(bs);
    bs >> mResourceDir;
}

void MiniLeaderboardDisplay::PostLoad(BinStream &bs) {
    mResourceDir.PostLoad(nullptr);
    UIComponent::PostLoad(bs);
    Update();
}

void MiniLeaderboardDisplay::DrawShowing() {
    if (mResourceDir) {
        mResourceDir->SetWorldXfm(WorldXfm());
        mResourceDir->DrawShowing();
    } else {
        MILO_NOTIFY_ONCE("MiniLeaderboardDisplay: %s missing resource dir", Name());
    }
}

void MiniLeaderboardDisplay::OldResourcePreload(BinStream &bs) {
    char name[256];
    bs.ReadString(name, 256);
    mResourceDir.SetName(name, true);
}

void MiniLeaderboardDisplay::Init() { REGISTER_OBJ_FACTORY(MiniLeaderboardDisplay); }
