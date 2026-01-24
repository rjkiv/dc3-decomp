#include "meta_ham/TexLoadPanel.h"
#include "macros.h"
#include "meta/SongMgr.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamSongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Utl.h"
#include "synth/Faders.h"
#include "synth/MoggClip.h"
#include "ui/PanelDir.h"
#include "ui/UIPanel.h"
#include "utl/FilePath.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region DynamicTex

DynamicTex::DynamicTex(const char *c1, const char *c2, bool b)
    : mTex(Hmx::Object::New<RndTex>()), mMatName(c2), mMat(0), mLoader(0) {
    if (c1 != gNullStr) {
        mLoader = dynamic_cast<FileLoader *>(TheLoadMgr.AddLoader(c1, kLoadFront));
        MILO_ASSERT(mLoader, 0x1a);
    }
    if (b) {
        mMat = Hmx::Object::New<RndMat>();
        mMat->SetZMode(kZModeDisable);
        mMat->SetBlend(BaseMaterial::kBlendSrcAlpha);
        CreateAndSetMetaMat(mMat);
    }
}

DynamicTex::~DynamicTex() {
    delete mMat;
    delete mLoader;
    delete mTex;
}

#pragma endregion DynamicTex
#pragma region DLCTex

void DLCTex::StartLoading() {
    MILO_ASSERT(mState == kMounting, 0x3d);
    const char *path = TheHamSongMgr.GetAlbumArtPath(unk18);
    MILO_ASSERT(path != gNullStr, 0x3f);
    mLoader = dynamic_cast<FileLoader *>(TheLoadMgr.AddLoader(path, kLoadFront));
    MILO_ASSERT(mLoader, 0x41);
    mState = 2;
}

#pragma endregion DLCTex
#pragma region TexLoadPanel

TexLoadPanel::TexLoadPanel() {
    mMoggClip = New<MoggClip>();
    mFader = New<Fader>();
    float fadeDbOffset = SystemConfig("sound")->FindFloat("voiceover_db_offset");
    mFader->SetVolume(fadeDbOffset);
    mMoggClip->AddFader(mFader);
    mMoggClip->UnloadWhenFinishedPlaying(true);
}

TexLoadPanel::~TexLoadPanel() {
    delete mFader;
    delete mMoggClip;
}

BEGIN_HANDLERS(TexLoadPanel)
    HANDLE_ACTION(add_tex, AddTex(_msg->Str(2), _msg->Str(3), false))
    HANDLE_ACTION(load_mogg_clip, LoadMoggClip(_msg->Str(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

void TexLoadPanel::Load() {
    mMoggClip->SetFile("");
    UIPanel::Load();
    if (RegisterForContent()) {
        TheContentMgr.RegisterCallback(this, false);
    }
}

void TexLoadPanel::Exit() {
    UIPanel::Exit();
    if (!mMoggClip->Path().empty()) {
        float fadeDuration = SystemConfig("sound")->FindFloat("voiceover_fade_duration");
        mMoggClip->FadeOut(fadeDuration);
    }
}

void TexLoadPanel::Poll() {
    UIPanel::Poll();
    DLCTex *dlc = NextDLCTex();
    if (dlc) {
        MILO_ASSERT(dlc->mState != DLCTex::kLoaded, 0x78);
        switch (dlc->mState) {
        case 0: {
            const char *c = TheHamSongMgr.ContentName(dlc->unk18, true);
            MILO_ASSERT(c, 0x7f);
            dlc->mState = 1;
            if (TheContentMgr.MountContent(c))
                dlc->StartLoading();
            break;
        }
        case 2:
            MILO_ASSERT(dlc->mLoader, 0x86);
            if (dlc->mLoader->IsLoaded()) {
                MILO_ASSERT(dlc->mTex, 0x89);
                MILO_ASSERT(dlc->mMat, 0x8a);
                dlc->mTex->SetBitmap(dlc->mLoader);
                dlc->mLoader = 0;
                dlc->mMat->SetDiffuseTex(dlc->mTex);
                dlc->mState = 3;
            }
            break;
        default:
            break;
        }
    }
}

bool TexLoadPanel::IsLoaded() const {
    if (!TexturesLoaded()) {
        return false;
    } else if (!mMoggClip->Path().empty() && !mMoggClip->IsReadyToPlay()) {
        return false;
    } else
        return UIPanel::IsLoaded();
}

void TexLoadPanel::Unload() {
    if (RegisterForContent()) {
        TheContentMgr.UnregisterCallback(this, false);
    }
    DeleteAll(mTexs);
    UIPanel::Unload();
}

void TexLoadPanel::FinishLoad() {
    UIPanel::FinishLoad();
    FinalizeTextures();
    if (!mMoggClip->Path().empty())
        mMoggClip->Play(0);
}

void TexLoadPanel::ContentMounted(const char *c1, const char *c2) {
    DLCTex *dlc = NextDLCTex();
    if (dlc) {
        String name(TheHamSongMgr.ContentName(dlc->unk18, true));
        if (name == c1)
            dlc->StartLoading();
        else
            MILO_NOTIFY("Someone else is mounting %s", c1);
    }
}

void TexLoadPanel::ContentFailed(const char *c1) {
    DLCTex *dlc = NextDLCTex();
    if (dlc) {
        String name(TheHamSongMgr.ContentName(dlc->unk18, true));
        if (name == c1) {
            dlc->mMat->SetDiffuseTex(dlc->unk24);
            dlc->mState = 3;
        } else
            MILO_NOTIFY("Someone else is mounting %s", c1);
    }
}

DLCTex *TexLoadPanel::NextDLCTex() {
    FOREACH (it, mTexs) {
        DLCTex *dlc = dynamic_cast<DLCTex *>(*it);
        if (dlc && dlc->mState != 3)
            return dlc;
    }
    return nullptr;
}

bool TexLoadPanel::TexturesLoaded() const {
    FOREACH (it, mTexs) {
        DynamicTex *tex = *it;
        if (tex->mLoader && !tex->mLoader->IsLoaded())
            return false;
    }
    return true;
}

DynamicTex *TexLoadPanel::AddTex(const char *c1, const char *c2, bool b) {
    DynamicTex *tex = new DynamicTex(c1, c2, b);
    mTexs.push_back(tex);
    return tex;
}

bool TexLoadPanel::RegisterForContent() const {
    const DataArray *tdef = TypeDef();
    if (tdef) {
        static Symbol register_for_content("register_for_content");
        DataArray *regArr = tdef->FindArray(register_for_content, false);
        if (regArr)
            return regArr->Int(1);
    }
    return false;
}

void TexLoadPanel::LoadMoggClip(const char *path) {
    MILO_ASSERT(mMoggClip->Path().empty(), 0x118);
    mMoggClip->SetFile(path);
}

void TexLoadPanel::FinalizeTextures() {
    FOREACH (it, mTexs) {
        DynamicTex *t = *it;
        if (t->mLoader) {
            MILO_ASSERT(t->mLoader->IsLoaded(), 0xf6);
            MILO_ASSERT(t->mTex, 0xf7);
            t->mTex->SetBitmap(t->mLoader);
            t->mLoader = 0;
            if (t->mMat) {
                t->mMat->SetDiffuseTex(t->mTex);
            } else {
                RndMat *found = mDir->Find<RndMat>(t->mMatName.c_str(), false);
                if (found)
                    found->SetDiffuseTex(t->mTex);
                else
                    MILO_NOTIFY("Could not find %s", t->mMatName);
            }
        }
    }
}

#pragma endregion TexLoadPanel
