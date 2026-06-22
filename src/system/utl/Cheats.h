#pragma once
#include "obj/Object.h"
#include "os/JoypadMsgs.h"
#include "os/Keyboard.h"
#include "obj/Data.h"
#include "os/User.h"

struct CheatLog {
    bool mQuick;
    int mPad;
    DataNode mScript;
};

struct KeyCheat {
    int mKey;
    bool mCtrl;
    bool mAlt;
    DataArray *mScript;
};
struct LongJoyCheat {
    LongJoyCheat() : ixProgress(0), mScript(nullptr) {}

    std::vector<int> mSequence;
    unsigned int ixProgress;
    DataArray *mScript;
};

struct QuickJoyCheat {
    int mButton;
    DataArray *mScript;
};

class CheatsManager : public Hmx::Object {
public:
    enum ShiftMode {
        kLeftShift = 0,
        kRightShift = 1,
        kDownShift = 2
    };

    CheatsManager();
    virtual DataNode Handle(DataArray *, bool);
    void AppendLog(FixedString &msg);

    Symbol CheatMode() { return mSymMode; }
    void SetKeyCheatsEnabled(bool b) { mKeyCheatsEnabled = b; };
    void Log(int padNum, bool quickCheat, DataArray *script);
    bool KeyCheatsEnabled() { return mKeyCheatsEnabled; };
    void CallCheatScript(
        bool quickCheat, DataArray *script, LocalUser *pLocalUser, bool require_joypad
    );
    void RebuildKeyCheatsForMode();
    void SetUnsafeCheatsUsed(bool b) { mUnsafeCheatsUsed = b; };
    void AddQuickJoyCheat(const QuickJoyCheat &cheat, ShiftMode mode) {
        mQuickJoyCheats[mode].push_back(cheat);
    }
    void AddKeyCheat(const KeyCheat &cheat) { mKeyCheats.push_back(cheat); }
    void AddLongJoyCheat(const LongJoyCheat &cheat) { mLongJoyCheats.push_back(cheat); }

    Symbol SymMode() { return mSymMode; }
    void SetSymMode(Symbol sym) {
        if (sym != mSymMode) {
            mSymMode = sym;
            RebuildKeyCheatsForMode();
        }
    }

private:
    int OnMsg(ButtonDownMsg const &);
    DataNode OnMsg(KeyboardKeyReleaseMsg const &);
    DataNode OnMsg(KeyboardKeyMsg const &);

protected:
    std::vector<LongJoyCheat> mLongJoyCheats; // 0x2c
    std::vector<QuickJoyCheat> mQuickJoyCheats[2]; // 0x38
    std::vector<KeyCheat> mKeyCheats; // 0x50
    Symbol mSymMode; // 0x5c
    std::vector<QuickJoyCheat *> mJoyCheatPtrsMode[2]; // 0x60
    std::vector<KeyCheat *> mKeyCheatPtrsMode; // 0x78
    Timer mLastButtonTime; // 0x88
    bool mKeyCheatsEnabled; // 0xb8
    bool mJoyCheatsEnabled; // 0xb9
    bool mUnlockAll; // 0xba
    std::list<CheatLog> mBuffer; // 0xbc
    int mMaxBuffer; // 0xc4
    bool mCtrlOverriddeMode; // 0xc8
    bool mIsOverridingKeyboard; // 0xc9
    Hmx::Object *mPreviousOverride; // 0xcc
    bool mUnsafeCheatsUsed; // 0xd0
    bool mDisplayCheats; // 0xd1
    // String mMessage; // 0xd4
    // float mMessageTimer; // 0xdc
};

extern CheatsManager *gCheatsManager;

void EnableKeyCheats(bool);
bool GetEnabledKeyCheats();
bool CheatsInitialized();
void CheatsInit();
void LogCheat(int padNum, bool quickCheat, DataArray *script);
void AppendCheatsLog(FixedString &msg);
void CallQuickCheat(DataArray *script, LocalUser *user);
void InitQuickJoyCheats(const DataArray *a, CheatsManager::ShiftMode);
void CheatsTerminate();
Symbol GetCheatMode();
