#include "Cheats.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/Keyboard.h"
#include "os/User.h"
#include "os/UserMgr.h"
#include "os/JoypadMsgs.h"
#include "os/Joypad.h"
#include "os/System.h"
#include "obj/DataFunc.h"

static bool sKeyCheatsEnabled = true;
static CheatsManager *gCheatsManager = nullptr;
static bool gDisable = false;

void InitQuickJoyCheats(const DataArray *cheats, CheatsManager::ShiftMode mode) {
    for (int i = 1; i < cheats->Size(); i++) {
        DataArray *curCheat = cheats->Array(i);
        if (curCheat->Int(0) >= 0 && curCheat->Int(0) < kPad_NumButtons) {
            QuickJoyCheat cheat;
            cheat.mButton = curCheat->Int(0);
            cheat.mScript = curCheat;
            gCheatsManager->AddQuickJoyCheat(cheat, mode);
        } else {
            MILO_LOG(
                "Error in quick_cheats: %s is not a valid button\n", curCheat->Str(0)
            );
        }
    }
}

void InitKeyCheats(const DataArray *cheats) {
    static Symbol ctrl("ctrl");
    static Symbol alt("alt");
    for (int i = 1; i < cheats->Size(); i++) {
        KeyCheat keyCheat;
        DataArray *cheat = cheats->Array(i);
        if (cheat->Type(0) == kDataInt) {
            if (cheat->Int(0) < 0) {
                MILO_LOG(
                    "Error in quick_cheats: %i is not a valid key code\n", cheat->Int(0)
                );
                continue;
            } else {
                keyCheat.mKey = cheat->Int(0);
            }
        } else {
            const char *key = cheat->Str(0);
            if (strlen(key) > 1) {
                MILO_LOG("Error in quick_cheats: %s is not a valid key\n", key);
                continue;
            } else {
                keyCheat.mKey = key[0];
            }
        }
        keyCheat.mAlt = false;
        keyCheat.mCtrl = false;
        int subIndex;
        for (subIndex = 1; subIndex < cheat->Size(); subIndex++) {
            if (cheat->Type(subIndex) != kDataSymbol) {
                break;
            }
            Symbol key = cheat->Sym(subIndex);
            if (key == ctrl) {
                keyCheat.mCtrl = true;
            } else if (key == alt) {
                keyCheat.mAlt = true;
            } else {
                MILO_NOTIFY("Unknown modifier symbol in cheat: %s", cheat->Sym(subIndex));
            }
        }
        MILO_ASSERT(cheat->Type( subIndex ) == kDataString, 0x2A6);
        keyCheat.mScript = cheat;
        gCheatsManager->AddKeyCheat(keyCheat);
    }
    gCheatsManager->RebuildKeyCheatsForMode();
}

void InitLongJoyCheats(const DataArray *cheats) {
    for (int i = 1; i < cheats->Size(); i++) {
        DataArray *cheat = cheats->Array(i);
        DataArray *buttons = cheat->Array(0);
        if (buttons->Size() > 16) {
            MILO_LOG("Too many buttons in long cheat, max %d\n", 16);
        } else {
            LongJoyCheat longJoyCheat;
            bool good = true;
            for (int j = 0; j < buttons->Size(); j++) {
                int button = buttons->Int(j);
                if (button >= 0 && button < kPad_NumButtons) {
                    longJoyCheat.mSequence.push_back(button);
                } else {
                    MILO_LOG("Error in long_cheats: %s is not a valid button\n", button);
                    good = false;
                    break;
                }
            }
            if (good) {
                longJoyCheat.mScript = cheat->Command(1);
                gCheatsManager->AddLongJoyCheat(longJoyCheat);
            }
        }
    }
}

DataNode OnGetCheatMode(DataArray *da) { return gCheatsManager->CheatMode(); }

DataNode OnSetCheatMode(DataArray *da) {
    gCheatsManager->SetSymMode(da->Sym(1));
    return 0;
}

DataNode SetKeyCheatsEnabled(DataArray *da) {
    bool result = da->Int(1) != 0;
    sKeyCheatsEnabled = result;
    if (gCheatsManager) {
        gCheatsManager->SetKeyCheatsEnabled(result);
    }
    return 0;
}

void EnableKeyCheats(bool b) {
    sKeyCheatsEnabled = b;
    if (gCheatsManager) {
        gCheatsManager->SetKeyCheatsEnabled(b);
    }
}

bool GetEnabledKeyCheats() { return sKeyCheatsEnabled; }
bool CheatsInitialized() { return gCheatsManager; }

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

void CallQuickCheat(DataArray *da, LocalUser *lu) {
    if (!gCheatsManager) {
        MILO_ASSERT(gCheatsManager, 0x309);
    }
    gCheatsManager->CallCheatScript(true, da, lu, false);
}

#pragma region CheatsManager

CheatsManager::CheatsManager()
    : mKeyCheatsEnabled(sKeyCheatsEnabled), mCtrlOverriddeMode(false),
      mIsOverridingKeyboard(false), mPreviousOverride(nullptr), mUnsafeCheatsUsed(false) {
    mLastButtonTime.Start();
    SystemConfig()->FindData("cheats_buffer", mMaxBuffer);
    DataArray *arr = SystemConfig()->FindArray("cheats_ctrl_mode", false);
    if (arr) {
        mCtrlOverriddeMode = arr->Int(1);
    }
    SetName("cheats_mgr", ObjectDir::Main());
}

BEGIN_HANDLERS(CheatsManager)
    HANDLE_ACTION(set_unsafe_cheat_used, mUnsafeCheatsUsed = true)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_MESSAGE(KeyboardKeyMsg)
    HANDLE_MESSAGE(KeyboardKeyReleaseMsg)
END_HANDLERS

void CheatsManager::AppendLog(FixedString &str) {
    char buffer[10];
    if (mBuffer.size() != 0) {
        str += "\n\nCheats Used";
        memcpy(buffer, "\n   %.30s", 10);
        FOREACH (it, mBuffer) {
            CheatLog &cur = *it;
            String curStr;
            cur.mScript.Print(curStr, true, 0);
            str += MakeString(buffer, curStr);
        }
        if (mBuffer.size() == mMaxBuffer) {
            str += "\n   ...";
        }
    }
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

void CheatsManager::RebuildKeyCheatsForMode() {
    static Symbol modes("modes");
    mKeyCheatPtrsMode.clear();
    FOREACH (it, mKeyCheats) {
        KeyCheat *cur = it;
        DataArray *modeArr = cur->mScript->FindArray(modes, false);
        if (!modeArr || modeArr->Contains(mSymMode)) {
            mKeyCheatPtrsMode.push_back(cur);
        }
    }
    for (int i = 0; i < 2; i++) {
        mJoyCheatPtrsMode[i].clear();
        FOREACH (it, mQuickJoyCheats[i]) {
            QuickJoyCheat *cur = it;
            DataArray *modeArr = cur->mScript->FindArray(modes, false);
            if (!modeArr || modeArr->Contains(mSymMode)) {
                mJoyCheatPtrsMode[i].push_back(cur);
            }
        }
    }
}

int CheatsManager::OnMsg(const ButtonDownMsg &msg) {
    User *user = msg.GetUser();
    LocalUser *lUser = nullptr;
    if (user) {
        lUser = user->GetLocalUser();
    }
    JoypadData *data = JoypadGetPadData(msg.GetPadNum());
    // for whatever reason, using data->Pressed generates extrwi's instead of rlwinm's
    // so screw it, we'll just write out the underlying inline code
    bool i6 = data->mButtons & 1 << kPad_Xbox_LB && data->mButtons & 1 << kPad_Xbox_LT;
    bool b9 = data->mButtons & 1 << kPad_Xbox_RB && data->mButtons & 1 << kPad_Xbox_RT;
    JoypadButton button = msg.GetButton();
    if (i6 || b9) {
        std::vector<QuickJoyCheat *> quickCheats = mJoyCheatPtrsMode[i6 == 0];
        FOREACH (it, quickCheats) {
            if (button == (*it)->mButton) {
                CallCheatScript(true, (*it)->mScript, lUser, true);
            }
        }
    }
    mLastButtonTime.Stop();
    if (mLastButtonTime.Ms() > 2000) {
        FOREACH (it, mLongJoyCheats) {
            it->ixProgress = 0;
        }
    }
    mLastButtonTime.Restart();
    if (!JoypadIsShiftButton(msg.GetPadNum(), button)) {
        FOREACH (it, mLongJoyCheats) {
            if (button == it->mSequence[it->ixProgress]) {
                it->ixProgress++;
                if (it->ixProgress >= it->mSequence.size()) {
                    CallCheatScript(false, it->mScript, lUser, true);
                    FOREACH (cheat, mLongJoyCheats) {
                        cheat->ixProgress = 0;
                    }
                    return 1;
                }
            } else {
                it->ixProgress = 0;
            }
        }
    }
    return 1;
}

DataNode CheatsManager::OnMsg(const KeyboardKeyReleaseMsg &msg) {
    if (msg->Int(2) == 0x11 && mIsOverridingKeyboard) {
        KeyboardOverride(mPreviousOverride);
        mIsOverridingKeyboard = false;
    }
    return 1;
}

DataNode CheatsManager::OnMsg(const KeyboardKeyMsg &msg) {
    if (!mKeyCheatsEnabled) {
        return DATA_UNHANDLED;
    } else {
        int key = msg.GetKey();
        if (key == 0x11 && mCtrlOverriddeMode) {
            if (!mIsOverridingKeyboard) {
                mPreviousOverride = KeyboardOverride(this);
                mIsOverridingKeyboard = true;
            }
            return 1;
        } else {
            std::vector<KeyCheat *> cheatPtrs(mKeyCheatPtrsMode);
            FOREACH (it, cheatPtrs) {
                KeyCheat *cur = *it;
                if (key == cur->mKey && msg.GetCtrl() == cur->mCtrl
                    && msg.GetAlt() == cur->mAlt) {
                    CallCheatScript(true, cur->mScript, nullptr, false);
                }
            }
            return 1;
        }
    }
}

#pragma endregion

void CheatsInit() {
    SystemConfig()->FindData("disable_cheats", gDisable);
    if (!gDisable) {
        MILO_ASSERT(gCheatsManager == null, 0x2d8);
        gCheatsManager = new CheatsManager();
        JoypadSubscribe(gCheatsManager);
        KeyboardSubscribe(gCheatsManager);

        DataArray *quickCheats = SystemConfig("quick_cheats");
        InitQuickJoyCheats(quickCheats->FindArray("left"), CheatsManager::kLeftShift);
        InitQuickJoyCheats(quickCheats->FindArray("right"), CheatsManager::kRightShift);

        InitKeyCheats(quickCheats->FindArray("keyboard"));

        InitLongJoyCheats(SystemConfig("long_cheats"));

        DataRegisterFunc("set_key_cheats_enabled", SetKeyCheatsEnabled);
        DataRegisterFunc("set_cheat_mode", OnSetCheatMode);
        DataRegisterFunc("get_cheat_mode", OnGetCheatMode);
    }
}

void CheatsTerminate() {
    if (!gDisable) {
        MILO_ASSERT(gCheatsManager, 0x2fa);
        JoypadUnsubscribe(gCheatsManager);
        KeyboardUnsubscribe(gCheatsManager);
        if (gCheatsManager) {
            delete gCheatsManager;
        }
        gCheatsManager = 0;
    }
}

Symbol GetCheatMode() { return gCheatsManager->CheatMode(); }
