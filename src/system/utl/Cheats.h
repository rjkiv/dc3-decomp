#pragma once

#include "os/JoypadMsgs.h"
#include "os/Keyboard.h"
#include "obj/Data.h"
#include "os/User.h"

bool gDisable;
bool gKeyCheatsEnabled;

struct CheatLog {
    ~CheatLog();
    bool mQuick;
    int mPad;
    DataNode mScript;
};

class CheatsManager : public Hmx::Object {
public:
    enum ShiftMode {
        kLeftShift = 0,
        kRightShift = 1,
        kDownShift = 2
    };
    struct KeyCheat {
        int mKey;
        bool mCtrl;
        bool mAlt;
        DataArray *mScript;
    };
    struct LongJoyCheat {
        std::vector<int> mSequence;
        unsigned int ixProgres;
        DataArray *mScript;
    };
    struct QuickJoyCheat {
        int mButton;
        DataArray *mScript;
    };


    CheatsManager();
    virtual ~CheatsManager();
    void AppendLog(FixedString &);
    virtual DataNode Handle(DataArray *, bool);

    Symbol CheatMode() { return mSymMode; }
    void setKeyCheatsEnabled(bool b) { mKeyCheatsEnabled = b; };
    void Log(int, bool, DataArray *);
    bool KeyCheatsEnabled() { return mKeyCheatsEnabled; };
    void CallCheatScript(bool b1, DataArray *da, LocalUser *lu, bool b2);
    void RebuildKeyCheatsForMode();
    void SetUnsafeCheatsUsed(bool b) { mUnsafeCheatsUsed = b; };

    Symbol GetSymMode() { return mSymMode; }
    void SetSymMode(Symbol sym) { mSymMode = sym; }

private:
    int OnMsg(ButtonDownMsg const &);
    DataNode OnMsg(KeyboardKeyReleaseMsg const &);
    DataNode OnMsg(KeyboardKeyMsg const &);

protected:
    std::vector<LongJoyCheat*> mLongJoyCheats; // 0x2c
    std::vector<QuickJoyCheat> mQuickJoyCheats[2]; // 0x50
    std::vector<KeyCheat> mKeyCheats;
    Symbol mSymMode; // 0x5c
    std::vector<QuickJoyCheat *> mJoyCheatPtrsMode[2];
    std::vector<KeyCheat *> mKeyCheatPtrsMode;
    Timer mLastButtonTime; // 0x88
    bool mKeyCheatsEnabled;
    bool mJoyCheatsEnabled;
    bool mUnlockAll;
    std::list<CheatLog> mBuffer; // 0xbc
    unsigned int mMaxBuffer;
    bool mCtrlOverriddeMode;
    bool mIsOverridingKeyboard;
    //MsgSinks *mPreviousOverride;

    bool mUnsafeCheatsUsed; // 0xd0
    bool mDisplayCheats;
    String mMessage;
    float mMessageTimer;

};

extern CheatsManager *gCheatsManager;

void EnableKeyCheats(bool);
bool GetEnabledKeyCheats();
bool CheatsInitialized();
void CheatsInit();
void LogCheat(int, bool, DataArray *);
void AppendCheatsLog(FixedString &);
void CallQuickCheat(DataArray *da, LocalUser *lu);
void InitQuickJoyCheats(const DataArray *a, CheatsManager::ShiftMode);
void CheatsTerminate();
DataNode OnGetCheatMode(DataArray *da);
DataNode SetKeyCheatsEnabled(DataArray *da);
DataNode OnSetCheatMode(DataArray *da);
Symbol GetCheatMode() { return gCheatsManager->CheatMode(); };
