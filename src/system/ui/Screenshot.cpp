#include "ui/Screenshot.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Utl.h"
#include "utl/Loader.h"
#include "rndobj/Rnd.h"

Screenshot::~Screenshot() {
    RELEASE(mTex);
    RELEASE(mMat);
}

Screenshot::Screenshot() : mTex(nullptr), mMat(nullptr) {}

BEGIN_PROPSYNCS(Screenshot)
    SYNC_PROP_MODIFY(tex_path, mTexPath, Sync())
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(Screenshot)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mTexPath;
END_SAVES

BEGIN_COPYS(Screenshot)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(Screenshot)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax)
            COPY_MEMBER(mTexPath)
    END_COPYING_MEMBERS
    Sync();
END_COPYS

BEGIN_LOADS(Screenshot)
    LOAD_REVS(bs);
    ASSERT_REVS(1, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    bs >> mTexPath;
    Sync();
END_LOADS

void Screenshot::DrawShowing() {
    if (!TheRnd.GetDrawMode() && TheLoadMgr.EditMode() && mMat) {
        TheRnd.DrawRect(
            Hmx::Rect(0, 0, TheRnd.Width(), TheRnd.Height()),
            Hmx::Color(0, 0, 0),
            mMat,
            0,
            0
        );
    }
}

void Screenshot::Sync() {
    if (TheLoadMgr.EditMode()) {
        RELEASE(mTex);
        RELEASE(mMat);
        mTex = Hmx::Object::New<RndTex>();
        mTex->SetBitmap(mTexPath);
        mMat = Hmx::Object::New<RndMat>();
        mMat->SetZMode(kZModeDisable);
        mMat->SetDiffuseTex(mTex);
        CreateAndSetMetaMat(mMat);
    }
}

BEGIN_HANDLERS(Screenshot)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
