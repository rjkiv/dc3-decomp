#include "lazer/game/GameMode.h"

bool (*g_LoaderModeCallback)(const Symbol &);

#include "char/FileMerger.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "macros.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/MetaPerformer.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/Symbol.h"

GameMode::GameMode() : mInPartyMode(0) {
    SetName("gamemode", ObjectDir::Main());
    SetMode("init", "none");
}

BEGIN_HANDLERS(GameMode)
    HANDLE(in_mode, OnInMode)
    HANDLE(set_mode, OnSetMode)
    HANDLE_ACTION(set_is_in_party_mode, SetInPartyMode(_msg->Int(2)))
    HANDLE_EXPR(is_in_party_mode, mInPartyMode)
    HANDLE_EXPR(is_infinite, mInfinite)
    HANDLE_EXPR(requires_two_players, RequiresTwoPlayers(_msg->Sym(2)))
    HANDLE_EXPR(get_mode, mMode)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool GameMode::InMode(Symbol sym, bool b) {
    if (mMode == sym)
        return true;
    if (b) {
        DataArray *sysConfig = SystemConfig("modes");
        Symbol s = mMode;
        static Symbol parent_mode("parent_mode");

        while (sysConfig->FindArray(s)->FindArray(parent_mode, false)) {
            s = sysConfig->FindArray(s)->FindArray(parent_mode)->Sym(1);
            if (s == sym)
                return true;
        }
    }
    return false;
}

DataNode GameMode::OnInMode(DataArray const *a) {
    MILO_ASSERT(a->Size() == 3 || a->Size() == 4, 0x6a);
    if (a->Size() == 3) {
        return InMode(a->Sym(2), true);
    } else {
        return InMode(a->Sym(2), a->Int(3));
    }
}

int GameMode::RequiresTwoPlayers(Symbol sym) {
    DataArray *sysConfig = SystemConfig("modes");
    DataArray *cloneArray = sysConfig->FindArray(sym, true)->Clone(true, false, 0);
    FillModeArrayWithParentData(sym, cloneArray, sysConfig);

    static Symbol requires_2_players("requires_2_players");
    DataArray *reqArray = cloneArray->FindArray(requires_2_players);
    int i = reqArray->Int(1);
    cloneArray->Release();
    return i;
}

int GameMode::MinPlayers(Symbol sym) {
    DataArray *sysConfig = SystemConfig("modes");
    DataArray *cloneArray = sysConfig->FindArray(sym, true)->Clone(true, false, 0);
    FillModeArrayWithParentData(sym, cloneArray, sysConfig);

    static Symbol min_players("min_players");

    DataArray *playerArray = cloneArray->FindArray(min_players);
    int i = playerArray->Int(1);
    cloneArray->Release();
    return i;
}

int GameMode::MaxPlayers(Symbol sym) {
    DataArray *sysConfig = SystemConfig("modes");
    DataArray *cloneArray = sysConfig->FindArray(sym, true)->Clone(true, false, 0);
    FillModeArrayWithParentData(sym, cloneArray, sysConfig);

    static Symbol max_players("max_players");

    DataArray *playerArray = cloneArray->FindArray(max_players);
    int i = playerArray->Int(1);
    cloneArray->Release();
    return i;
}

void GameMode::SetMode(Symbol mode, Symbol s2) {
    if (mMode != mode) {
        DataArray *cfg = SystemConfig("modes");
        static Message exitMsg("exit");
        HandleType(exitMsg);
        mMode = mode;
        DataArray *cloned = cfg->FindArray(mMode)->Clone(true, false, 0);
        static Symbol parent_only("parent_only");
        if (cloned->FindArray(parent_only, false)) {
            if (cloned->FindArray(parent_only)->Int(1)) {
                MILO_FAIL("Trying to set mode %s, which is a parent_only mode!\n", mMode);
            }
        }
        static Symbol parent_mode("parent_mode");
        for (Symbol s = mMode; cfg->FindArray(s)->FindArray(parent_mode, false);) {
            s = cfg->FindArray(s)->FindArray(parent_mode)->Sym(1);
            DataMergeTags(cloned, cfg->FindArray(s));
        }
        DataMergeTags(cloned, cfg->FindArray("defaults"));
        SetTypeDef(cloned);
        cloned->Release();
        static Symbol metamode("metamode");
        TheHamProvider->SetProperty(metamode, mMode);
        static Symbol gameplay_mode("gameplay_mode");
        TheHamProvider->SetProperty(gameplay_mode, cloned->FindSym(gameplay_mode));
        static Symbol none("none");
        static Symbol microgame("microgame");
        static Symbol battle_mode("battle_mode");
        static Symbol is_in_campaign_mode("is_in_campaign_mode");
        static Symbol campaign("campaign");
        mBattleMode = s2 == none ? Property(battle_mode)->Sym() : s2;
        TheHamProvider->SetProperty(microgame, mBattleMode);
        SetProperty(battle_mode, mBattleMode);
        mParentMode = Property("parent_mode")->Sym();
        mParentOnly = Property("parent_only")->Int();
        mGameplayMode = Property("gameplay_mode")->Sym();
        mCanLose = Property("can_lose")->Int();
        mPauseCountIn = Property("pause_count_in")->Int();
        mRequires2Players = Property("requires_2_players")->Int();
        mCrowdReacts = Property("crowd_reacts")->Int();
        mLoadChars = Property("load_chars")->Int();
        mUseStaticTip = Property("use_static_tip")->Int();
        mRanked = Property("ranked")->Int();
        mUpdateLeaderboards = Property("update_leaderboards")->Int();
        mInfinite = Property("infinite")->Int();
        mMinPlayers = Property("min_players")->Int();
        mMaxPlayers = Property("max_players")->Int();
        {
            int val = Property("is_in_timeywimey")->Int();
            TheGameData->SetInTimeyWimey(val);
        }
        if (mode == Symbol("campaign_outro")) {
            MetaPerformer::Current()->OnLoadSong();
        }
        if (parent_mode == campaign) {
            TheHamProvider->SetProperty(is_in_campaign_mode, true);
        }
        static Message enterMsg("enter");
        HandleType(enterMsg);
    }
}

void GameMode::SetGameplayMode(Symbol s, bool b) {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol merge_moves("merge_moves");
    static Symbol use_movegraph("use_movegraph");

    SetProperty(gameplay_mode, s);
    mGameplayMode = s;
    TheHamProvider->SetProperty(merge_moves, b);
    TheHamProvider->SetProperty(use_movegraph, b);
    TheGameMode->SetProperty(merge_moves, b);
    TheGameMode->SetProperty(use_movegraph, b);
    static Message load_game_hud("load_game_hud", 0, 0, 0, 0);
    FileMerger *fm = TheHamDirector->GetGameModeMerger();
    fm->HandleType(load_game_hud);
    fm->StartLoad(true);
}

DataNode GameMode::OnSetMode(const DataArray *a) {
    MILO_ASSERT(a->Size() == 3 || a->Size() == 4, 0x78);
    if (a->Size() == 3) {
        SetMode(a->Sym(2), "none");
    } else {
        SetMode(a->Sym(2), a->Sym(3));
    }
    return 0;
}

void GameMode::FillModeArrayWithParentData(Symbol sym, DataArray *a1, DataArray *a2) {
    if (!a2) {
        a2 = SystemConfig("modes");
    }
    static Symbol parent_mode("parent_mode");
    for (Symbol s = sym; a2->FindArray(s)->FindArray(parent_mode, false);) {
        s = a2->FindArray(s)->FindArray(parent_mode)->Sym(1);
        DataMergeTags(a1, a2->FindArray(s));
    }
    DataMergeTags(a1, a2->FindArray("defaults"));
}

bool IsInLoaderMode(const Symbol &sym) {
    if (TheGameMode && TheGameMode->InMode(sym, true))
        return true;

    static Symbol mind_control("mind_control");
    static Symbol era05("era05");

    if (sym == mind_control) {
        if (TheGameMode->InMode("campaign", true)) {
            CampaignPerformer *pPerformer =
                dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
            MILO_ASSERT(pPerformer, 0x28);
            if (pPerformer->Era() == era05
                && !pPerformer->IsCampaignMindControlComplete()) {
                return true;
            }
        }
    }
    return false;
}

void GameModeInit() {
    MILO_ASSERT(TheGameMode == NULL, 0x35);
    TheGameMode = new GameMode();
    g_LoaderModeCallback = IsInLoaderMode;
}

void GameModeTerminate() {
    RELEASE(TheGameMode);
    TheGameMode = nullptr;
}

bool GameMode::IsGameplayModePerform() const {
    static Symbol perform("perform");
    return mGameplayMode == perform;
}
