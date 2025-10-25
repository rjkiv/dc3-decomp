#include "lazer/game/GameMode.h"
#include "flow/PropertyEventProvider.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/Symbol.h"

GameMode::GameMode() : unk6c(0) {
    SetName("gamemode", ObjectDir::Main());
    SetMode("init", "none");
}

bool GameMode::InMode(Symbol sym, bool b) {
    if (unk2c == sym)
        return true;
    if (b) {
        DataArray *sysConfig = SystemConfig("modes");
        Symbol s = unk2c;
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
    }
    return NULL_OBJ;
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

void GameMode::SetMode(Symbol sym1, Symbol sym2) {
    if (unk2c != sym1) {
        DataArray *arr = SystemConfig("modes");
        static Symbol exit("exit");
        HandleType(Message(exit));
        unk2c = sym1;

        DataArray *cloneArray = arr->Clone(true, false, 0);
        static Symbol parent_only("parent_only");
        if (cloneArray->FindArray(parent_only, false)) {
            if (cloneArray->FindArray(parent_only, true)->Int(1)) {
                MILO_FAIL("Trying to set mode %s, which is a parent_only mode!\n", unk2c);
            }
        }

        static Symbol parent_mode("parent_mode");
        Symbol sym = unk2c;
        while (arr->FindArray(sym)->FindArray(parent_mode, false)) {
            sym = arr->FindArray(sym)->FindArray(parent_mode)->Sym(1);
            DataMergeTags(cloneArray, arr->FindArray(sym));
        }

        DataMergeTags(cloneArray, arr->FindArray("defaults"));
        SetTypeDef(cloneArray);
        cloneArray->Release();

        static Symbol metamode("metamode");

        unk38 = Property("parent_mode", true)->Int(0);
        unk3c = Property("gameplay_mode", true)->Sym(0);
        unk40 = Property("can_lose", true)->Int(0);
        unk44 = Property("pause_count_in", true)->Int(0);
        unk48 = Property("requires_two_players", true)->Int(0);
        unk4c = Property("crowd_reacts", true)->Int(0);
        unk50 = Property("load_chars", true)->Int(0);
        unk54 = Property("use_static_tip", true)->Int(0);
        unk58 = Property("ranked", true)->Int(0);
        unk5c = Property("update_leaderboards", true)->Int(0);
        unk60 = Property("infinite", true)->Int(0);
        unk64 = Property("min_players", true)->Int(0);
        unk68 = Property("max_players", true)->Int(0);
        unk6c = Property("is_in_timeywimey", true)->Int(0);

        if (sym1 == "campaign_outro") {
            // MetaPerformer::Current()->
        }

        static Symbol enter("enter");
        HandleType(Message(enter));
    }
}

void GameMode::SetGameplayMode(Symbol s, bool b) {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol merge_moves("merge_moves");
    static Symbol use_movegraph("use_movegraph");

    SetProperty(gameplay_mode, s);
}

DataNode GameMode::OnSetMode(DataArray const *a) {
    Symbol s;
    MILO_ASSERT(a->Size() == 3 || a->Size() == 4, 0x78);

    if (a->Size() == 3) {
        s = "none";
    } else {
        s = a->Sym(3);
    }

    SetMode(a->Sym(2), s);
    return DataNode(0);
}

void GameMode::FillModeArrayWithParentData(Symbol sym, DataArray *a1, DataArray *a2) {
    if (a2 == nullptr) {
        a2 = SystemConfig("modes");
    }

    static Symbol parent_mode("parent_mode");
}

BEGIN_HANDLERS(GameMode)
END_HANDLERS

void GameModeTerminate() {
    RELEASE(TheGameMode);
    TheGameMode = nullptr;
}

void GameModeInit() { MILO_ASSERT(TheGameMode == NULL, 0x35); }

bool IsInLoaderMode(const Symbol &sym) {
    if (TheGameMode && TheGameMode->InMode(sym.Str(), true))
        return true;

    static Symbol mind_control("mind_control");
    static Symbol era05("era05");

    if (sym == mind_control) {
        if (TheGameMode->InMode("campaign", true)) {
        }
    }
    return false;
}
