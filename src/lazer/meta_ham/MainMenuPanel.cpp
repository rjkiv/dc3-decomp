#include "meta_ham/MainMenuPanel.h"
#include "HamPanel.h"
#include "HamProfile.h"
#include "HamSongMgr.h"
#include "MainMenuPanel.h"
#include "ProfileMgr.h"
#include "hamobj/HamLabel.h"
#include "macros.h"
#include "math/Rand.h"
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
#include "rndobj/Text.h"
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

MainMenuPanel::MotdData::MotdData() : length(0) {}

MainMenuPanel::MotdData::MotdData(MotdData const &motdData)
    : type(motdData.type), text(motdData.text), length(motdData.length) {}

MainMenuPanel::MainMenuPanel()
    : mMsgLabel(), mNeedToLoadNetCacheMgr(false), mNeedToCleanupNetCacheMgr(false),
      mDlcArt(), mUtilityArt(), mIsDlcArtLoaded(false), mIsUtilityArtLoaded(false),
      mIsMiscArtLoaded(false), mMotdUpdatedNeeded(false), mMotdFreq(), mCurrMotdFreq(),
      mStatsDisplayLimit(), mCurrStatsAccumulated(), mCommunityDisplayLimit(),
      mCurrCommunityAccumulated(), mSettings() {}

MainMenuPanel::~MainMenuPanel() { DeleteDownloadedArts(); }

MainMenuProvider *MainMenuPanel::GetMainMenuProvider() { return &mProvider; }

BEGIN_HANDLERS(MainMenuPanel)
    HANDLE_ACTION(
        update_main_menu_provider, mProvider.UpdateList(_msg->Obj<UIListProvider>(2))
    )
    HANDLE_EXPR(get_main_menu_provider, GetMainMenuProvider())
    HANDLE_EXPR(dlc_image, mDlcArt)
    HANDLE_EXPR(utility_image, mUtilityArt)
    HANDLE_ACTION(update_icon_state, UpdateIconState(_msg->Sym(2)))
    HANDLE_ACTION(motd_setup, MotdSetup(_msg->Obj<HamLabel>(2)))
    HANDLE_ACTION(download_motd_art, DownloadMotdArt())
    HANDLE_ACTION(text_scrolled_in, MotdHandleTextScrolledIn(_msg->Int(2)))
    HANDLE_ACTION(text_scrolled_out, MotdHandleTextScrolledOut(_msg->Int(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

BEGIN_PROPSYNCS(MainMenuPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void MainMenuPanel::Load() {
    UIPanel::Load();
    TheContentMgr.StartRefresh();
    mNeedToCleanupNetCacheMgr = false;
    mNeedToLoadNetCacheMgr = true;
    mIsDlcArtLoaded = false;
    mIsUtilityArtLoaded = false;
    mIsMiscArtLoaded = false;
    DeleteDownloadedArts();
    mDlcArt = New<RndTex>();
    mUtilityArt = New<RndTex>();
}

void MainMenuPanel::Enter() {
    HamPanel::Enter();
    if (mNeedToLoadNetCacheMgr) {
        TheNetCacheMgr->Load((NetCacheMgr::CacheSize)1);
        mNeedToLoadNetCacheMgr = false;
        mNeedToCleanupNetCacheMgr = true;
    }
    TheContentMgr.RegisterCallback(this, true);
}

void MainMenuPanel::Exit() {
    UIPanel::Exit();
    mMessages.clear();
    mMotdData.clear();
    mMsgLabel = 0;
    mMotdUpdatedNeeded = false;
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
    if (mNeedToCleanupNetCacheMgr)
        CleanupNetCacheRelated();
    DeleteDownloadedArts();
    UIPanel::Unload();
}

void MainMenuPanel::FinishLoad() {
    UIPanel::FinishLoad();
    mSettings = DataDir()->Find<PropertyEventProvider>("player.ep", true);
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
        mSettings->SetProperty(state, s);
    } else {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        if (pProfile) {
            mSettings->SetProperty(state, profile);
            mSettings->SetProperty(rank, pProfile->GetRank()->RankNumber());
            mSettings->SetProperty(tier, pProfile->GetRank()->GetTier());
            mSettings->SetProperty(gamertag, pProfile->GetName());
        } else {
            mSettings->SetProperty(state, no_profile);
        }
    }
    Flow *f = DataDir()->Find<Flow>("udpate_icon_state.flow", true);
    f->Activate();
}

void MainMenuPanel::CleanupNetCacheRelated() {
    FOREACH (it, mLoaders)
        TheNetCacheMgr->DeleteNetCacheLoader(*it);
    mLoaders.clear();
    TheNetCacheMgr->Unload();
    mNeedToCleanupNetCacheMgr = false;
}

void MainMenuPanel::ContentDone() { HandleType(Message("content_refresh_Done")); }

void MainMenuPanel::DownloadMotdArt() {
    if (mNeedToLoadNetCacheMgr) {
        TheNetCacheMgr->Load((NetCacheMgr::CacheSize)1);
        mNeedToCleanupNetCacheMgr = true;
        mNeedToLoadNetCacheMgr = false;
    }
    mIsDlcArtLoaded = true;
    mIsUtilityArtLoaded = true;
    mIsMiscArtLoaded = true;
}

void MainMenuPanel::DeleteDownloadedArts() {
    if (mDlcArt) {
        RELEASE(mDlcArt);
    }
    if (mUtilityArt) {
        RELEASE(mUtilityArt);
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
    mMessages.clear();
    HamProfile *activeProfile = TheProfileMgr.GetActiveProfile(true);
    if (activeProfile) {
        if (TheRockCentral.HasDlcMsg()) {
            String msg;
            TheRockCentral.GetDlcMsg(msg);
            mMessages[dlc].push_back(msg);
        }
        if (TheRockCentral.HasUtilityMsg()) {
            String msg;
            TheRockCentral.GetUtilityMsg(msg);
            mMessages[utility].push_back(msg);
        }
        int commMsgCount = TheRockCentral.GetCommunityMsgCount();
        for (int i = 0; i < commMsgCount; i++) {
            String msg;
            TheRockCentral.GetCommunityMsg(i, msg);
            mMessages[community].push_back(msg);
        }

        MetagameStats *playerStats = activeProfile->GetStats();
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
                mMessages[stats].push_back(stat);
            }
            if (TheContentMgr.RefreshDone()) {
                static Symbol stat_curr_library_size("stat_curr_library_size");
                int totalNumSongs = TheHamSongMgr.GetTotalNumLibrarySongs();
                String retval = MakeString(
                    Localize(stat_curr_library_size, false, TheLocale),
                    LocalizeSeparatedInt(totalNumSongs, TheLocale)
                );
                mMessages[stats].push_back(retval);
            }
        } else {
            static Symbol stat_welcome("stat_welcome");
            String locale = Localize(stat_welcome, false, TheLocale);
            mMessages[stats].push_back(locale);
        }
    } else {
        static Symbol message_noprofile("message_noprofile");
        String locale = Localize(message_noprofile, false, TheLocale);
        mMessages[no_profile].push_back(locale);
    }
    mMsgLabel = label;
    MotdInitializeTexts();
}

void MainMenuPanel::MotdHandleTextScrolledIn(int i) {
    static Symbol dlc("dlc");
    static Symbol utility("utility");
    if (mMotdUpdatedNeeded) {
        auto begin = mMotdData.begin();
        std::advance(begin, i);

        if (begin->type == dlc) {
            Sound *s = DataDir()->Find<Sound>("motd_store_item_new.snd", false);
            if (s) {
                s->Play(0, 0, 0, nullptr, 0);
            }
            UpdateIconState(begin->type);
        } else if (begin->type == utility) {
            Sound *s =
                DataDir()->Find<Sound>(TheRockCentral.GetUtilitySound().c_str(), false);
            if (s) {
                s->Play(0, 0, 0, nullptr, 0);
            }
            UpdateIconState(begin->type);
        }
    }
}

void MainMenuPanel::MotdHandleTextScrolledOut(int) {
    static Symbol dlc("dlc");
    static Symbol utility("utility");
    static Symbol none("none");
    if (mMotdUpdatedNeeded) {
        Symbol type = mMotdData.front().type;
        if (type == dlc || type == utility) {
            UpdateIconState(none);
        }

        mMotdData.pop_front();
        float f = 0.0f;
        float width = mMsgLabel->Width() * 2.0f;
        auto motdIt = mMotdData.begin();
        if (motdIt == mMotdData.end()) {
            MotdPickNextText();
        } else {
            for (++motdIt; motdIt != mMotdData.end(); ++motdIt) {
                f += motdIt->length;
            }
        }
        while (f < width) {
            f += MotdPickNextText();
        }
        MILO_ASSERT(mMotdData.size(), 0x301);
        String text = mMotdData.front().text;
        for (auto it = ++mMotdData.begin(); it != mMotdData.end(); ++it) {
            text += "\n";
            text += it->text;
        }
        mMsgLabel->ReFitTextScroll(text);
    }
}

void MainMenuPanel::UpdateArtLoaders() {
    if (TheNetCacheMgr->HasFailed()) {
        HandleNetCacheMgrFailure();
        if (mNeedToCleanupNetCacheMgr) {
            CleanupNetCacheRelated();
        }
        mNeedToLoadNetCacheMgr = true;
    } else if (TheNetCacheMgr->IsReady()) {
        if (mIsDlcArtLoaded) {
            mIsDlcArtLoaded = false;
            LoadArt(TheRockCentral.GetDlcImage());
        }
        if (mIsUtilityArtLoaded) {
            mIsUtilityArtLoaded = false;
            LoadArt(TheRockCentral.GetUtilityImage());
        }
        if (mIsMiscArtLoaded) {
            mIsMiscArtLoaded = false;
            LoadArt(TheRockCentral.GetMiscImage());
        }
        for (auto it = mLoaders.begin(); it != mLoaders.end();) {
            NetCacheLoader *loader = *it;
            if (loader->IsLoaded()) {
                int size = loader->GetSize();
                char *buffer = loader->GetBuffer();
                MILO_ASSERT(buffer, 0x10d);
                RndBitmap bitmap;
                BufStream stream(buffer, size, true);
                bitmap.Load(stream);
                bitmap.SetMip(nullptr);
                TheNetCacheMgr->DeleteNetCacheLoader(loader);
                if (TheRockCentral.GetDlcImage() == loader->GetRemotePath()) {
                    mDlcArt->SetBitmap(bitmap, nullptr, false, RndTex::kRegular);
                    if (mState == 1) {
                        static Message dlc_image_loaded("dlc_image_loaded");
                        Handle(dlc_image_loaded, false);
                    }
                }
                if (TheRockCentral.GetUtilityImage() == loader->GetRemotePath()) {
                    mUtilityArt->SetBitmap(bitmap, nullptr, false, RndTex::kRegular);
                    if (mState == 1) {
                        static Message utility_image_loaded("utility_image_loaded");
                        Handle(utility_image_loaded, false);
                    }
                }
                if (TheRockCentral.GetMiscImage() == loader->GetRemotePath()) {
                    TheRockCentral.SetMiscArtBitMap(bitmap);
                }
                // mLoaders.pop_front();
                it = mLoaders.erase(it);
            } else if (loader->HasFailed()) {
                NetCacheMgrFailType failType = loader->GetFailType();
                TheNetCacheMgr->DeleteNetCacheLoader(loader);
                // mLoaders.pop_front();
                it = mLoaders.erase(it);
                HandleNetCacheLoaderFailure(failType);
            } else {
                ++it;
            }
        }
    }
}

void MainMenuPanel::MotdInitializeTexts() {
    static Symbol dlc("dlc");
    static Symbol utility("utility");
    static Symbol community("community");
    static Symbol stats("stats");
    static Symbol no_profile("no_profile");
    if (mMsgLabel->GetFitType() != RndText::FitType::kFitScrollMarqueeWrapAlways) {
        MILO_LOG(
            ">>>>>>>>>> Forcing the souce lable to use kFitScrollMarqueeWrapAlways as the text fit type.\n"
        );
        mMsgLabel->SetFitType(RndText::FitType::kFitScrollMarqueeWrapAlways);
    }
    mMsgLabel->SetUnk78(nullptr);
    mMotdUpdatedNeeded = false;
    mMotdData.clear();
    if (!mMessages[no_profile].empty()) {
        mMsgLabel->SetPrelocalizedString(mMessages[no_profile].front());
    } else if (mMessages[dlc].empty() && mMessages[utility].empty()
               && mMessages[community].size() + mMessages[stats].size() == 1) {
        mMsgLabel->SetPrelocalizedString(mMessages[stats].front());
    } else {
        mMotdUpdatedNeeded = true;
        mMsgLabel->SetUnk78(this);

        float width = mMsgLabel->Width() * 2.0f;
        mMotdFreq = TheRockCentral.GetMotdFreq();
        int commStatSize = mMessages[community].size() + mMessages[stats].size();
        if (mMessages[dlc].empty() && mMessages[utility].empty()) {
            mMotdFreq = 0;
        } else if (mMotdFreq < 1) {
            mMotdFreq = 1;
        } else if (commStatSize < mMotdFreq - 1) {
            mMotdFreq = commStatSize + 1;
        }
        if (mMessages[community].size() == 0) {
            mCommunityDisplayLimit = 0;
        } else if (mMessages[community].size() > 1) {
            mCommunityDisplayLimit = 2;
        }
        if (mMessages[stats].size() > 1) {
            mStatsDisplayLimit = 2;
        } else {
            mStatsDisplayLimit = 1;
        }

        mCurrStatsAccumulated = 0;
        mCurrCommunityAccumulated = 0;
        mCurrMotdFreq = 0;
        float f = 0.0f;
        prevMotd = utility;
        MotdPickNextText();
        while (f < width) {
            f += MotdPickNextText();
        }
        MILO_ASSERT(mMotdData.size(), 600);
        String text = mMotdData.front().text;
        for (auto it = ++mMotdData.begin(); it != mMotdData.end(); ++it) {
            text += "\n";
            text += it->text;
        }
        mMsgLabel->SetPrelocalizedString(text);
    }
}

void MainMenuPanel::LoadArt(String s) {
    if (s == gNullStr) {
        return;
    }

    auto it = mLoaders.begin();
    for (; it != mLoaders.end(); ++it) {
        if (s == (*it)->GetRemotePath()) {
            return;
        }
    }
    if (it == mLoaders.end()) {
        NetCacheLoader *pLoader =
            TheNetCacheMgr->AddNetCacheLoader(s.c_str(), (NetLoaderPos)0);
        if (pLoader) {
            mLoaders.push_back(pLoader);
        }
    }
}

float MainMenuPanel::MotdPickNextText() {
    MILO_ASSERT(mMsgLabel, 0x269);
    static Symbol dlc("dlc");
    static Symbol utility("utility");
    static Symbol community("community");
    static Symbol stats("stats");
    MotdData motdData;
    if (mMotdFreq != 0 && (mCurrMotdFreq % mMotdFreq) == 0) {
        Symbol key = prevMotd == utility ? dlc : utility;
        motdData.type = key;
        motdData.text = mMessages[key].front();
        motdData.length =
            mMsgLabel->ComputeCharWidthsForText(motdData.text) + mMsgLabel->Indentation();
        prevMotd = key;
        mCurrMotdFreq = 1;
    } else {
        Symbol key;
        if (mCommunityDisplayLimit == 0
            || mCurrCommunityAccumulated >= mCommunityDisplayLimit) {
            key = stats;
            mCurrCommunityAccumulated = 0;
            mCurrStatsAccumulated = 1;
        } else if (mCurrStatsAccumulated >= mStatsDisplayLimit) {
            key = community;
            mCurrCommunityAccumulated = 1;
            mCurrStatsAccumulated = 0;
        } else if (RandomInt(0, 2) != 0) {
            key = community;
            mCurrStatsAccumulated = 0;
            mCurrCommunityAccumulated++;
        } else {
            key = stats;
            mCurrCommunityAccumulated = 0;
            mCurrStatsAccumulated++;
        }
        int rand = 0;
        motdData.type = key;
        if (mMessages[key].size() > 1) {
            rand = RandomInt(0, mMessages[key].size() / 2);
        }
        auto it = mMessages[key].begin();
        if (rand > 0) {
            for (; rand != 0; rand--) {
                ++it;
            }
            motdData.text = *it;
        } else {
            for (; rand != 0; rand++) {
                --it;
            }
            motdData.text = *it;
        }
        motdData.length =
            mMsgLabel->ComputeCharWidthsForText(motdData.text) + mMsgLabel->Indentation();
        mMessages[key].push_back(*it);
        mMessages[key].erase(it);
        if (mMotdFreq != 0) {
            mCurrMotdFreq++;
        }
    }
    mMotdData.push_back(motdData);
    return mMotdData.back().length;
}
