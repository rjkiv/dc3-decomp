#include "meta_ham/MainMenuPanel.h"
#include "HamPanel.h"
#include "HamProfile.h"
#include "MainMenuPanel.h"
#include "ProfileMgr.h"
#include "hamobj/HamLabel.h"
#include "macros.h"
#include "meta_ham/MainMenuProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "rndobj/Tex.h"
#include "ui/UIListProvider.h"
#include "ui/UIPanel.h"
#include "utl/NetCacheMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

#pragma region MotdData

MainMenuPanel::MotdData::MotdData() : unkc(0) {}

MainMenuPanel::MotdData::MotdData(MotdData const &motdData)
    : unk0(motdData.unk0), unk4(motdData.unk4), unkc(motdData.unkc) {}

#pragma endregion MotdData
#pragma region MainMenuPanel

MainMenuPanel::MainMenuPanel()
    : unk40(), unk80(false), unk81(false), unk8c(), unk90(), unk94(false), unk95(false),
      unk96(false), unkb0(false), unkbc(), unkc0(), unkc4(), unkc8(), unkcc(), unkd0(),
      unkd8() {}

MainMenuPanel::~MainMenuPanel() { DeleteDownloadedArts(); }

BEGIN_PROPSYNCS(MainMenuPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void MainMenuPanel::Load() {
    UIPanel::Load();
    TheContentMgr.StartRefresh();
    unk81 = false;
    unk80 = true;
    unk94 = false;
    unk95 = false;
    unk96 = false;
    DeleteDownloadedArts();
    unk8c = New<RndTex>();
    unk90 = New<RndTex>();
}

void MainMenuPanel::Enter() {
    HamPanel::Enter();
    if (unk80) {
        TheNetCacheMgr->Load((NetCacheMgr::CacheSize)1);
        unk80 = false;
        unk81 = true;
    }
    TheContentMgr.RegisterCallback(this, true);
}

void MainMenuPanel::Exit() {
    UIPanel::Exit();
    unk98.clear();
    unkb4.clear();
    unk40 = 0;
    unkb0 = false;
    TheContentMgr.UnregisterCallback(this, true);
}

bool MainMenuPanel::Unloading() const {
    if (mState != 1 && !TheNetCacheMgr->IsUnloaded())
        return true;
    else
        return UIPanel::Unloading();
}

void MainMenuPanel::Poll() {
    HamPanel::Poll();
    UpdateArtLoaders();
}

void MainMenuPanel::Unload() {
    if (unk81)
        CleanupNetCacheRelated();
    DeleteDownloadedArts();
    UIPanel::Unload();
}

void MainMenuPanel::FinishLoad() {
    UIPanel::FinishLoad();
    unkd8 = DataDir()->Find<PropertyEventProvider>("player.ep", true);
}

void MainMenuPanel::UpdateIconState(Symbol s) {
    static Symbol dlc("dlc");
    static Symbol utility("utility");
    static Symbol no_profile("no_profile");
    static Symbol profile("profile");
    static Symbol state("state");
    static Symbol rank("rank");
    static Symbol tier("tier");
    static Symbol gamertag("gamertag");
    if (s == dlc || s == utility) {
        unkd8->SetProperty(state, s);
    } else {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        if (pProfile) {
            unkd8->SetProperty(state, profile);
            unkd8->SetProperty(rank, pProfile->GetMetagameRank()->RankNumber());
            unkd8->SetProperty(tier, pProfile->GetMetagameRank()->GetTier());
            unkd8->SetProperty(gamertag, pProfile->GetName());
        } else {
            unkd8->SetProperty(state, no_profile);
        }
    }
    Flow *f = DataDir()->Find<Flow>("udpate_icon_state.flow", true);
    f->Activate();
}

void MainMenuPanel::CleanupNetCacheRelated() {
    FOREACH (it, unk84)
        TheNetCacheMgr->DeleteNetCacheLoader(*it);
    unk84.clear();
    TheNetCacheMgr->Unload();
    unk81 = false;
}

void MainMenuPanel::ContentDone() { HandleType(Message("content_refresh_Done")); }

void MainMenuPanel::DownloadMotdArt() {
    if (unk80) {
        TheNetCacheMgr->Load((NetCacheMgr::CacheSize)1);
        unk81 = true;
        unk80 = false;
    }
    unk94 = true;
    unk95 = true;
    unk96 = true;
}

void MainMenuPanel::DeleteDownloadedArts() {
    if (unk8c) {
        delete unk8c;
        unk8c = nullptr;
    }
    if (unk90) {
        delete unk90;
        unk90 = nullptr;
    }
}

BEGIN_HANDLERS(MainMenuPanel)
    HANDLE_ACTION(
        update_main_menu_provider, unk44.UpdateList(_msg->Obj<UIListProvider>(2))
    )
    HANDLE_EXPR(get_main_menu_provider, &unk44) // not a perfect match for some reason
    HANDLE_EXPR(dlc_image, unk8c)
    HANDLE_EXPR(utility_image, unk90)
    HANDLE_ACTION(update_icon_state, UpdateIconState(_msg->Sym(2)))
    HANDLE_ACTION(motd_setup, MotdSetup(_msg->Obj<HamLabel>(2)))
    HANDLE_ACTION(download_motd_art, DownloadMotdArt())
    HANDLE_ACTION(text_scrolled_in, MotdHandleTextScrolledIn(_msg->Int(2)))
    HANDLE_ACTION(text_scrolled_out, MotdHandleTextScrolledOut(_msg->Int(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

#pragma endregion MainMenuPanel
