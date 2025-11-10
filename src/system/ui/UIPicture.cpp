#include "ui/UIPicture.h"
#include "UIComponent.h"
#include "UITransitionHandler.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/File.h"
#include "rndobj/Mesh.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include <cstring>

UIPicture::UIPicture()
    : UITransitionHandler(this), mMesh(this), mTexFile(), mLoadedFile(),
      mTex(Hmx::Object::New<RndTex>()), mLoader(0), mHookTex(true), mDelayedTexFile("") {}

UIPicture::~UIPicture() {
    CancelLoading();
    delete mTex;
}

BEGIN_PROPSYNCS(UIPicture)
    SYNC_PROP_SET(tex_file, mTexFile, SetTex(FilePath(_val.Str())))
    SYNC_PROP_SET(in_anim, GetInAnim(), SetInAnim(_val.Obj<RndAnimatable>()))
    SYNC_PROP_SET(out_anim, GetOutAnim(), SetOutAnim(_val.Obj<RndAnimatable>()))
    SYNC_PROP_MODIFY(mesh, mMesh, HookupMesh())
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(UIPicture)
    SAVE_REVS(2, 0)
    bs << mTexFile;
    bs << mMesh;
    SaveHandlerData(bs);
    SAVE_SUPERCLASS(UIComponent)
END_SAVES

BEGIN_COPYS(UIPicture)
    CREATE_COPY_AS(UIPicture, p)
    MILO_ASSERT(p, 0x38);
    UIComponent::Copy(p, ty);
    COPY_MEMBER_FROM(p, mMesh)
    UITransitionHandler::CopyHandlerData(p);
END_COPYS

BEGIN_LOADS(UIPicture)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void UIPicture::SetTypeDef(DataArray *da) {
    UIComponent::SetTypeDef(da);
    if (da) {
        DataArray *findtex = da->FindArray("tex_file", false);
        if (findtex) {
            if (strlen(findtex->Str(1))) {
                FilePath fp(FilePath(FileGetPath(findtex->File()), findtex->Str(1)));
                SetTex(fp);
            }
        }
    }
}

void UIPicture::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    if (d.rev > 0) {
        if (TheLoadMgr.EditMode()) {
            FilePath fp;
            bs >> fp;
            SetTex(fp);
        } else {
            bs >> mTexFile;
        }
        bs >> mMesh;
    }
    if (d.rev >= 2)
        UITransitionHandler::LoadHandlerData(bs);
    UIComponent::PreLoad(bs);
}

void UIPicture::PostLoad(BinStream &bs) {
    UIComponent::PostLoad(bs);
    CancelLoading();
    if (!TheLoadMgr.EditMode() && mMesh) {
        mMesh->SetShowing(false);
    }
}

void UIPicture::Poll() {
    UIComponent::Poll();
    if (mDelayedTexFile != "") {
        UpdateTexture(mDelayedTexFile);
        mDelayedTexFile.SetRoot("");
    }
    UpdateHandler();
}

void UIPicture::Exit() {
    UIComponent::Exit();
    CancelLoading();
}

void UIPicture::SetTex(FilePath const &p) {
    if (HasTransitions() || (!(p == mLoadedFile) || !(p == mTexFile))) {
        if (TheLoadMgr.EditMode()) {
            mDelayedTexFile = p;
        } else
            UpdateTexture(p);
    }
}

void UIPicture::SetHookTex(bool b) {
    mHookTex = b;
    if (b == 0)
        return;
    mLoadedFile.SetRoot("");
}

void UIPicture::FinishValueChange() {
    if (mLoader && mLoader->IsLoaded()) {
        FinishLoading();
        UITransitionHandler::FinishValueChange();
    } else if (mMesh)
        mMesh->SetShowing(false);
}

bool UIPicture::IsEmptyValue() const { return mTexFile == ""; }

void UIPicture::CancelLoading() {
    if (mLoader) {
        delete mLoader;
        mLoader = 0;
    }
}

void UIPicture::HookupMesh() {
    if (mMesh && mHookTex) {
        RndMat *mat = mMesh->Mat();
        if (mat) {
            RndTex *tex = mTex;
            if (tex && tex->Width() != 0 && tex->Height() != 0) {
                mat->SetDiffuseTex(tex);
            } else {
                mat->SetDiffuseTex(0);
            }
        } else {
            if (mLoader || TheLoadMgr.EditMode()) {
                MILO_NOTIFY("%s does not have material", mMesh->Name());
            }
        }
        mMesh->SetShowing(true);
    }
}

void UIPicture::FinishLoading() {
    MILO_ASSERT(mLoader, 0xe2);
    MILO_ASSERT(mLoader->IsLoaded(), 0xe3);
    mTex->SetBitmap(mLoader);
    HookupMesh();
    mLoader = NULL;
    mLoadedFile = mTexFile;
}

void UIPicture::UpdateTexture(FilePath const &p) {
    mTexFile = p;
    CancelLoading();
    if (mTexFile != "") {
        if (!strstr(mTexFile.c_str(), "_keep"))
            MILO_NOTIFY("%s will not be included on a disc build", p);
        mLoader = dynamic_cast<FileLoader *>(TheLoadMgr.AddLoader(mTexFile, kLoadFront));
        MILO_ASSERT(mLoader, 0xda);
    }
    UITransitionHandler::StartValueChange();
}

BEGIN_HANDLERS(UIPicture)
    HANDLE_EXPR(tex, mTex)
    HANDLE_ACTION(set_hook_tex, SetHookTex(_msg->Int(2)))
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS
