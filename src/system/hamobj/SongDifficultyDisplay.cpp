#include "hamobj/SongDifficultyDisplay.h"
#include "SongDifficultyDisplay.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "ui/UIComponent.h"
#include "utl/BinStream.h"

SongDifficultyDisplay::SongDifficultyDisplay() : mAnimation(this), mResourceDir(this) {}
SongDifficultyDisplay::~SongDifficultyDisplay() {}

BEGIN_HANDLERS(SongDifficultyDisplay)
    HANDLE_ACTION(set_level, SetLevel(_msg->Int(2)))
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_PROPSYNCS(SongDifficultyDisplay)
    SYNC_PROP_MODIFY(resource, mResourceDir, Update())
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(SongDifficultyDisplay)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(UIComponent)
    bs << mResourceDir;
END_SAVES

BEGIN_COPYS(SongDifficultyDisplay)
    COPY_SUPERCLASS(UIComponent)
    CREATE_COPY(SongDifficultyDisplay)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mResourceDir)
    END_COPYING_MEMBERS
    Update();
END_COPYS

BEGIN_LOADS(SongDifficultyDisplay)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void SongDifficultyDisplay::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    UIComponent::PreLoad(bs);
    if (gRev >= 2) {
        bs >> mResourceDir;
    }
}

void SongDifficultyDisplay::PostLoad(BinStream &bs) {
    mResourceDir.PostLoad(nullptr);
    UIComponent::PostLoad(bs);
    Update();
}

void SongDifficultyDisplay::DrawShowing() {
    MILO_ASSERT(mResourceDir, 0x69);
    mResourceDir->SetWorldXfm(WorldXfm());
    mResourceDir->Draw();
}

void SongDifficultyDisplay::OldResourcePreload(BinStream &bs) {
    char name[256];
    bs.ReadString(name, 256);
    mResourceDir.SetName(name, true);
}

void SongDifficultyDisplay::Init() { REGISTER_OBJ_FACTORY(SongDifficultyDisplay); }

void SongDifficultyDisplay::SetLevel(int level) {
    MILO_ASSERT(mAnimation.Ptr(), 0x60);
    MILO_ASSERT(level >= 0 && level <= 6, 0x61);
    mAnimation->SetFrame(level, 1);
}

void SongDifficultyDisplay::Update() {
    RndAnimatable *anim;
    if (mResourceDir) {
        anim = mResourceDir->Find<RndAnimatable>("difficulty.anim", false);
    } else {
        anim = nullptr;
    }
    mAnimation = anim;
}
