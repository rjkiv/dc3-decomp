#include "hamobj/StarsDisplay.h"
#include "StarsDisplay.h"
#include "hamobj/HamLabel.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIColor.h"
#include "ui/UIComponent.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

StarsDisplay::StarsDisplay()
    : mResourceDir(this), mStarsLabel(Hmx::Object::New<HamLabel>()),
      mDiffLabel(Hmx::Object::New<HamLabel>()),
      mNoFlashcardsLabel(Hmx::Object::New<HamLabel>()), mShowUnplayedSong(0), mAlpha(1) {}

StarsDisplay::~StarsDisplay() {
    delete mStarsLabel;
    delete mDiffLabel;
    delete mNoFlashcardsLabel;
}

BEGIN_HANDLERS(StarsDisplay)
    HANDLE_ACTION(set_stars, SetStars(_msg->Int(2)))
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_PROPSYNCS(StarsDisplay)
    SYNC_PROP_MODIFY(resource, mResourceDir, Update())
    SYNC_PROP(show_unplayed_song, mShowUnplayedSong)
    SYNC_PROP(alpha, mAlpha)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(StarsDisplay)
    SAVE_REVS(3, 0)
    bs << mAlpha;
    SAVE_SUPERCLASS(UIComponent)
    bs << mResourceDir;
END_SAVES

BEGIN_COPYS(StarsDisplay)
    COPY_SUPERCLASS(UIComponent)
    CREATE_COPY(StarsDisplay)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mAlpha)
        COPY_MEMBER(mResourceDir)
    END_COPYING_MEMBERS
    Update();
END_COPYS

BEGIN_LOADS(StarsDisplay)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void StarsDisplay::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    if (gRev >= 2) {
        bs >> mAlpha;
    }
    UIComponent::PreLoad(bs);
    if (gRev >= 3) {
        bs >> mResourceDir;
    }
    bsrev.PushRev(this);
}

void StarsDisplay::PostLoad(BinStream &bs) {
    bs.PopRev(this);
    mResourceDir.PostLoad(nullptr);
    UIComponent::PostLoad(bs);
    Update();
}

void StarsDisplay::OldResourcePreload(BinStream &bs) {
    char name[256];
    bs.ReadString(name, 256);
    mResourceDir.SetName(name, true);
}

void StarsDisplay::SetAlphaColor(float alpha, UIColor *) { mAlpha = alpha; }

void StarsDisplay::DrawShowing() {
    mStarsLabel->Style(0).SetAlpha(mAlpha);
    mDiffLabel->Style(0).SetAlpha(mAlpha);
    mNoFlashcardsLabel->Style(0).SetAlpha(mAlpha);
    if (mStarsLabel->Showing()) {
        mStarsLabel->DrawShowing();
    }
    if (mDiffLabel->Showing()) {
        mDiffLabel->DrawShowing();
    }
    if (mNoFlashcardsLabel->Showing()) {
        mNoFlashcardsLabel->DrawShowing();
    }
}

void StarsDisplay::Init() { REGISTER_OBJ_FACTORY(StarsDisplay); }

void StarsDisplay::Update() {
    MILO_ASSERT(mResourceDir, 0x2C);
    mStarsLabel->Copy(mResourceDir->Find<HamLabel>("stars.lbl", true), kCopyShallow);
    mStarsLabel->SetTransParent(this, false);
    mDiffLabel->Copy(mResourceDir->Find<HamLabel>("diff.lbl", true), kCopyShallow);
    mDiffLabel->SetTransParent(this, false);
    mNoFlashcardsLabel->Copy(
        mResourceDir->Find<HamLabel>("no_flashcards.lbl", true), kCopyShallow
    );
    mNoFlashcardsLabel->SetTransParent(this, false);
    if (!TheLoadMgr.EditMode()) {
        mStarsLabel->SetShowing(false);
        mDiffLabel->SetShowing(false);
        mNoFlashcardsLabel->SetShowing(false);
    }
}

void StarsDisplay::SetStars(int stars) {
    String str("stars_");
    str += stars + 0x30; // take the int and ascii-ify it
    mStarsLabel->SetTextToken(str.c_str());
    mStarsLabel->SetShowing(true);
}
