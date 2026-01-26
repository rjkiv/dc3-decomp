#include "Cheats.h"

#include "os/UserMgr.h"
#include "os/JoypadMsgs.h"
#include "os/Joypad.h"
#include "os/System.h"
#include "obj/DataFunc.h"

void InitQuickJoyCheats(const DataArray *a, CheatsManager::ShiftMode);
void InitKeyCheats(const DataArray *);
void InitLongJoyCheats(const DataArray *);


bool CheatsInitialized() {
    return gCheatsManager != 0;
}

BEGIN_HANDLERS(CheatsManager)
HANDLE_ACTION(set_unsafe_cheat_used, gCheatsManager->mUnsafeCheatsUsed)
HANDLE_MESSAGE(ButtonDownMsg)
HANDLE_MESSAGE(KeyboardKeyMsg)
HANDLE_MESSAGE(KeyboardKeyReleaseMsg)
END_HANDLERS

void CheatsInit() {
    SystemConfig()->FindData("disable_cheats", gDisable, true);
    if (!gDisable) {
        if (gCheatsManager != 0) {
            MILO_ASSERT(gCheatsManager == null, 0x2d8);
        }
        gCheatsManager = new CheatsManager();
        JoypadSubscribe(gCheatsManager);
        KeyboardSubscribe(gCheatsManager);

        DataArray *quickCheats = SystemConfig("quick_cheats");
        InitQuickJoyCheats(quickCheats->FindArray("left", true), CheatsManager::kLeftShift);
        InitQuickJoyCheats(quickCheats->FindArray("right", true), CheatsManager::kRightShift);

        InitKeyCheats(quickCheats->FindArray("keyboard", true));

        InitLongJoyCheats(SystemConfig("long_cheats"));

        DataRegisterFunc("set_key_cheats_enabled", SetKeyCheatsEnabled);
        DataRegisterFunc("set_cheat_mode", OnSetCheatMode);
        DataRegisterFunc("get_cheat_mode", OnGetCheatMode);
    }
}

DataNode OnGetCheatMode(DataArray *da) {
    return gCheatsManager->CheatMode();
}

void EnableKeyCheats(bool b) {
    gKeyCheatsEnabled = b;
    if (gCheatsManager) {
        gCheatsManager->setKeyCheatsEnabled(b);
    }

}

DataNode OnSetCheatMode(DataArray *da) {
    Symbol sym = da->Sym(1);
    if (gCheatsManager->GetSymMode() != sym) {
        gCheatsManager->SetSymMode(sym);
        gCheatsManager->RebuildKeyCheatsForMode();
    }
    return 0;
}

DataNode SetKeyCheatsEnabled(DataArray *da) {
    bool result = da->Int(1) != 0;
    gKeyCheatsEnabled = result;
    if (gCheatsManager) {
        gCheatsManager->setKeyCheatsEnabled(result);
    }
    return 0;
}

CheatsManager::~CheatsManager() {  }

CheatsManager::CheatsManager() : mLongJoyCheats(0), mSymMode(gNullStr), mBuffer(0) {
    SetName("cheats_mgr", ObjectDir::Main());
}

void CheatsManager::AppendLog(FixedString &fs) {
    FOREACH(it, mBuffer) {
        fs += "\n\nCheats Used";
        String s = "\n   %.30s";
        //memcpy(s, "\n   %.30s", 10);
        it->mScript.Print(s, true, 0);
        const char *c = MakeString("\n   %.30s");
        fs += c;
    }
    if (mBuffer.size() == mMaxBuffer) {
        fs += "\n   ...";
    }
}

void LogCheat(int i1, bool b, DataArray *da) {
    if (!gCheatsManager) {
        MILO_ASSERT(gCheatsManager, 0x303);
    }
    gCheatsManager->Log(i1, b, da);
}

void AppendCheatsLog(FixedString &fs) {
    if (gCheatsManager) {
        gCheatsManager->AppendLog(fs);
    }
}

bool GetEnabledKeyCheats() {
    return gKeyCheatsEnabled;
}

void CheatsManager::Log(int padNum, bool quickCheat, DataArray *script) {
    CheatLog log;
    log.mPad = padNum;
    log.mQuick = quickCheat;
    log.mScript = script;
    mBuffer.push_front(log);

    if (mBuffer.size() > mMaxBuffer) {
        mBuffer.pop_back();
    }
}

void CheatsManager::CallCheatScript(bool b1, DataArray *da, LocalUser *lu, bool b2) {
    if (!lu) {
        if (TheUserMgr) {
            std::vector<LocalUser *> users;
            TheUserMgr->GetLocalUsers(users);
            for (int i = 0; i < (int)users.size(); i++) {
                CallCheatScript(b1, da, users[i], b2);
            }
        }
    } else {
        Log(lu->GetPadNum(), b1, da);
    }

}

void CallQuickCheat(DataArray *da, LocalUser *lu) {
    if (!gCheatsManager) {
        MILO_ASSERT(gCheatsManager, 0x309);
    }
    gCheatsManager->CallCheatScript(true, da, lu, false);
}

void CheatsTerminate() {
    if (!gDisable) {
        if (!gCheatsManager) {
            MILO_ASSERT(gCheatsManager, 0x2fa);
        }
        JoypadUnsubscribe(gCheatsManager);
        KeyboardUnsubscribe(gCheatsManager);
        if (gCheatsManager) {
            delete gCheatsManager;
        }
        gCheatsManager = 0;
    }
}

void CheatsManager::RebuildKeyCheatsForMode() {
    return;
}


CheatLog::~CheatLog() {}

DataNode CheatsManager::OnMsg(KeyboardKeyReleaseMsg const &msg) {
    if (msg->Int(2) == 0x11 && mIsOverridingKeyboard)

    return 0;
}