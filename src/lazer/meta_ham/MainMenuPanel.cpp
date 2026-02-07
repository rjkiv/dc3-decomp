#include "meta_ham/MainMenuPanel.h"
#include "HamPanel.h"
#include "HamProfile.h"
#include "HamSongMgr.h"
#include "MainMenuPanel.h"
#include "ProfileMgr.h"
#include "hamobj/HamLabel.h"
#include "macros.h"
#include "meta_ham/MainMenuProvider.h"
#include "meta_ham/MetaPanel.h"
#include "meta_ham/MetagameStats.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "synth/Sound.h"
#include "ui/UIListProvider.h"
#include "ui/UIPanel.h"
#include "utl/BufStream.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"
#include "utl/NetCacheLoader.h"
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
    : mMsgLabel(), unk80(false), unk81(false), unk8c(), unk90(), unk94(false),
      unk95(false), unk96(false), unkb0(false), unkbc(), unkc0(), unkc4(), unkc8(),
      unkcc(), unkd0(), unkd8() {}

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
    mMotdData.clear();
    mMsgLabel = 0;
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

void MainMenuPanel::HandleNetCacheMgrFailure() {
    NetCacheMgrFailType failType = TheNetCacheMgr->GetFailType();
    switch (failType) {
    case kNCMFT_StoreServer:
        break;
    case kNCMFT_ClientError:
        MILO_LOG("[MainMenuPanel::HandleNetCacheMgrFailure] kNCMFT_ClientError.\n");
        break;
    case kNCMFT_NoEthernetCable:
        MILO_LOG("[MainMenuPanel::HandleNetCacheMgrFailure] kNCMFT_NoEthernetCable.\n");
        break;
    default:
        MILO_NOTIFY("Unknown failure %d in NetCacheMgr.", failType);
        break;
    }
}

void MainMenuPanel::HandleNetCacheLoaderFailure(int failType) {
    MILO_ASSERT_RANGE(failType, 0, kNCMFT_Max, 0x166);
    if (failType == kNCMFT_Unknown)
        failType = TheNetCacheMgr->GetFailType();

    switch (failType) {
    case kNCMFT_StoreServer:
        MILO_LOG("[MainMenuPanel::HandleNetCacheLoaderFailure] kNCMFT_StoreServer.\n");
        break;
    case kNCMFT_ClientError:
        break;
    case kNCMFT_NoEthernetCable:
        MILO_LOG("[MainMenuPanel::HandleNetCacheLoaderFailure] kNCMFT_NoEthernetCable.\n");
        break;
    default:
        MILO_NOTIFY("Unknown failure %d in a net cache loader!", failType);
        break;
    }
}

void MainMenuPanel::MotdSetup(HamLabel *label) {
    MILO_ASSERT(label, 0x183);
    static Symbol dlc("dlc");
    static Symbol utility("utility");
    static Symbol community("community");
    static Symbol stats("stats");
    static Symbol no_profile("no_profile");
    unk98.clear();
    HamProfile *activeProfile = TheProfileMgr.GetActiveProfile(true);
    if (activeProfile) {
        if (TheRockCentral.HasDlcMsg()) {
            String msg;
            TheRockCentral.GetDlcMsg(msg);
            unk98[dlc].push_back(msg);
        }
        if (TheRockCentral.HasUtilityMsg()) {
            String msg;
            TheRockCentral.GetUtilityMsg(msg);
            unk98[utility].push_back(msg);
        }
        int commMsgCount = TheRockCentral.GetCommunityMsgCount();
        for (int i = 0; i < commMsgCount; i++) {
            String msg;
            TheRockCentral.GetCommunityMsg(i, msg);
            unk98[community].push_back(msg);
        }

        MetagameStats *playerStats = activeProfile->GetMetagameStats();
        MILO_ASSERT(playerStats, 0x1a9);
        int timesPlayed =
            playerStats->GetCount(MetagameStats::kCountStat_TimesPlayedPerform);
        timesPlayed +=
            playerStats->GetCount(MetagameStats::kCountStat_TimesPlayedMultiplayer);
        timesPlayed +=
            playerStats->GetCount(MetagameStats::kCountStat_TimesPlayedPractice);
        int numData = playerStats->NumData();
        if (MetaPanel::sMotdCheat || timesPlayed >= 10) {
            for (int i = 0; i < numData; i++) {
                String stat;
                playerStats->InqStatString(i, stat);
                unk98[stats].push_back(stat);
            }
            if (TheContentMgr.RefreshDone()) {
                static Symbol stat_curr_library_size("stat_curr_library_size");
                int totalNumSongs = TheHamSongMgr.GetTotalNumLibrarySongs();
                String retval = MakeString(
                    Localize(stat_curr_library_size, false, TheLocale),
                    LocalizeSeparatedInt(totalNumSongs, TheLocale)
                );
                unk98[stats].push_back(retval);
            }
        } else {
            static Symbol stat_welcome("stat_welcome");
            String locale = Localize(stat_welcome, false, TheLocale);
            unk98[stats].push_back(locale);
        }
    } else {
        static Symbol message_noprofile("message_noprofile");
        String locale = Localize(message_noprofile, false, TheLocale);
        unk98[no_profile].push_back(locale);
    }
    mMsgLabel = label;
    MotdInitializeTexts();
}

void MainMenuPanel::MotdHandleTextScrolledOut(int i) {
    static Symbol dlc("dlc");
    static Symbol utility("utility");
    static Symbol none("none");
    if (!unkb0) {
        return;
    }
    Symbol type = mMotdData.front().unk0;
    if (type == dlc || type == utility) {
        UpdateIconState(none);
    }

    mMotdData.pop_front();
    float f = 0.0f;
    float width = mMsgLabel->Width() * 2.0f;
    if (mMotdData.empty()) {
        MotdPickNextText();
    } else {
        FOREACH (it, mMotdData) {
            f += (*it).unkc;
        }
    }
    while (f < width) {
        f += MotdPickNextText();
    }
    MILO_ASSERT(mMotdData.size(), 0x301);
    String text = mMotdData.front().unk4;
    FOREACH (it, mMotdData) {
        text += "\n";
        text += (*it).unk4;
    }
    mMsgLabel->ReFitTextScroll(text);
}

void MainMenuPanel::UpdateArtLoaders() {
    if (TheNetCacheMgr->GetUnk30()) {
        HandleNetCacheMgrFailure();
        if (unk81) {
            CleanupNetCacheRelated();
        }
        unk80 = true;
    } else {
        if (TheNetCacheMgr->IsReady()) {
            if (unk94) {
                unk94 = false;
                LoadArt(TheRockCentral.GetDLCImage());
            }
            if (unk95) {
                unk95 = false;
                LoadArt(TheRockCentral.GetUtilityImage());
            }
            if (unk96) {
                unk96 = false;
                LoadArt(TheRockCentral.GetMiscImage());
            }
            FOREACH (it, unk84) {
                NetCacheLoader *loader = *it;
                if (loader->IsLoaded()) {
                    int size = loader->GetSize();
                    char *buffer = loader->GetBuffer();
                    MILO_ASSERT(buffer, 0x10d);
                    RndBitmap bitmap;
                    BufStream stream = BufStream(loader, size, true);
                    bitmap.Load(stream);
                    bitmap.SetMip(nullptr);
                    TheNetCacheMgr->DeleteNetCacheLoader(loader);
                    if (TheRockCentral.GetDLCImage() == loader->GetRemotePath()) {
                        unk8c->SetBitmap(bitmap, nullptr, false, RndTex::kRegular);
                        if (mState == 1) {
                            static Message dlc_image_loaded("dlc_image_loaded");
                            Handle(dlc_image_loaded, false);
                        }
                    }
                    if (TheRockCentral.GetUtilityImage() == loader->GetRemotePath()) {
                        unk90->SetBitmap(bitmap, nullptr, false, RndTex::kRegular);
                        if (mState == 1) {
                            static Message utility_image_loaded("utility_image_loaded");
                            Handle(utility_image_loaded, false);
                        }
                    }
                    if (TheRockCentral.GetMiscImage() == loader->GetRemotePath()) {
                        TheRockCentral.SetMiscArtBitMap(bitmap);
                    }
                    unk84.pop_front();
                } else {
                    if (loader->HasFailed()) {
                        NetCacheMgrFailType failType = loader->GetFailType();
                        TheNetCacheMgr->DeleteNetCacheLoader(loader);
                        unk84.pop_front();
                        HandleNetCacheLoaderFailure(failType);
                    }
                }
            }
        }
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
