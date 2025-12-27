#include "game/PresenceMgr.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/HamUI.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "utl/Symbol.h"
#include "utl/UTF8.h"

void PresenceMgr::Init() {
    static Symbol presence_mgr("presence_mgr");
    DataArray *presenceArray = SystemConfig()->FindArray(presence_mgr, false);
    if (presenceArray) {
        static Symbol presence_modes("presence_modes");
        DataArray *presenceModesArray = presenceArray->FindArray(presence_modes, false);
        unk2c = presenceModesArray;
        if (presenceModesArray) {
            static Symbol presence_mode_contexts("presence_mode_contexts");
            DataArray *presenceModeContextArray =
                presenceArray->FindArray(presence_mode_contexts);
            unk30 = presenceModeContextArray;
            static Symbol instrument_play_mode_contexts("instrument_play_mode_contexts");
            DataArray *instrumentPlayModeContextsArray =
                presenceArray->FindArray(instrument_play_mode_contexts);
            unk34 = instrumentPlayModeContextsArray;
        }
    }

    if (unk2c) {
        static Symbol signin_changed("signin_changed");
        ThePlatformMgr.AddSink(this, signin_changed);
        TheHamUI.AddSink(this, CurrentScreenChangedMsg::Type());
        HamPlayerData *pPlayer1 = TheGameData->Player(0);
        MILO_ASSERT(pPlayer1, 0x48);
        PropertyEventProvider *pProvider1 = pPlayer1->Provider();
        MILO_ASSERT(pProvider1, 0x4a);
        HamPlayerData *pPlayer2 = TheGameData->Player(1);
        MILO_ASSERT(pPlayer2, 0x4c);
        PropertyEventProvider *pProvider2 = pPlayer2->Provider();
        MILO_ASSERT(pProvider2, 0x4e);
        static Symbol on_player_present_change("on_player_present_change");
        pProvider1->AddSink(this, on_player_present_change, gNullStr, kHandle, false);
        pProvider2->AddSink(this, on_player_present_change, gNullStr, kHandle, false);
    }
}

void PresenceMgr::UpdatePresence() {
    if (unk2c) {
        Symbol presenceMode = GetPresenceMode();
        for (int i = 0; i < 4; i++) {
            if (ThePlatformMgr.IsSignedIn(i)) {
                int presenceContext =
                    GetPresenceContextFromMode(presenceMode, IsPadPlaying(i));
                ThePlatformMgr.SetPadPresence(i, presenceContext);
                int playModeContext = GetPlayModeContext();
                ThePlatformMgr.SetPadContext(i, 11, playModeContext);
                const char *s = " ";
                if (mSongID != 0) {
                    const HamSongMetadata *hsm = TheHamSongMgr.Data(mSongID);
                    s = hsm->Title();
                }
                ThePlatformMgr.SetPadProperty(i, 0x40000003, CharToWideChar(s));
            }
        }
    }
}

int PresenceMgr::GetPlayModeContext() {
    if (!unk2c)
        return -1;
    else {
        static Symbol Default("default");
        DataArray *defaultArray = unk34->FindArray(Default);
        static Symbol learn("learn");
        static Symbol multiplayer("multiplayer");
        static Symbol party("party");
        static Symbol throwdown("throwdown");
        static Symbol challenge("challenge");
        static Symbol play("play");
        static Symbol none("none");
        static Symbol practice("practice");
        static Symbol dance_battle("dance_battle");
        static Symbol is_in_infinite_party_mode("is_in_infinite_party_mode");
        static Symbol is_in_party_mode("is_in_party_mode");
        static Symbol perform("perform");
        if (!TheGameMode->InMode(practice, true)) {
            const DataNode *isInPartyNode =
                TheHamDirector->Property(is_in_party_mode, true);
            if (isInPartyNode->Int() == 0) {
                const DataNode *isInInfPartyNode =
                    TheHamDirector->Property(is_in_infinite_party_mode, true);
            }
        }
    }
}

DataNode PresenceMgr::OnPlayerPresentChange(DataArray *) {
    UpdatePresence();
    return DataNode(0);
}

DataNode PresenceMgr::OnPresenceChange(DataArray *) {
    if (!unk2c)
        return 0;
    else {
        UpdatePresence();
        return 0;
    }
}

void PresenceMgr::SetNotInGame() {
    if (!unk2c) {
        return;
    }
    mInGame = false;
    mSongID = 0;
    UpdatePresence();
}

void PresenceMgr::SetInGame(int id) {
    if (!unk2c)
        return;
    mSongID = id;
    mInGame = true;
    UpdatePresence();
}

int PresenceMgr::GetPresenceContextFromMode(Symbol s, bool b) {
    if (!unk2c)
        return -1;
    else {
        DataArray *presenceArray = unk30->FindArray(s, true);
        return presenceArray->Int(b ? 1 : 2);
    }
}

bool PresenceMgr::IsPadPlaying(int pad) {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x9a);
        int padNum = pPlayer->PadNum();
        if (pPlayer->IsPlaying() && padNum == pad)
            return true;
    }
    return false;
}

BEGIN_HANDLERS(PresenceMgr)
    HANDLE(current_screen_changed, OnPresenceChange)
    HANDLE(signin_changed, OnPresenceChange)
    HANDLE(on_player_present_change, OnPlayerPresentChange)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
