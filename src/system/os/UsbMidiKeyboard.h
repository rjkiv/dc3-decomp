#pragma once
#include "os/Joypad.h"

class UsbMidiKeyboard {
public:
    UsbMidiKeyboard();
    ~UsbMidiKeyboard();
    static void Init();
    static void Terminate();
    static void Poll();

    int GetAccelAxisVal(int, int);
    bool GetKeyPressed(int, int);
    int GetKeyVelocity(int, int);
    void SetAccelerometer(int, int, int, int);
    void SetSustain(int, bool);
    void SetStompPedal(int, bool);
    void SetModVal(int, int);
    void SetExpressionPedal(int, int);
    void SetConnectedAccessories(int, int);
    void SetLowHandPlacement(int, int);
    void SetHighHandPlacement(int, int);
    void SetKeyPressed(int, int, bool);
    void SetKeyVelocity(int, int, int);

    bool GetSustain(int i) { return mSustain[i]; }
    bool GetStompPedal(int i) { return mStompPedal[i]; }
    int GetModVal(int i) { return mModVal[i]; }
    int GetExpressionPedal(int i) { return mExpressionPedal[i]; }
    int GetConnectedAccessory(int i) { return mConnectedAccessories[i]; }
    int GetLowHandPlacement(int i) { return mLowHandPlacement[i]; }
    int GetHighHandPlacement(int i) { return mHighHandPlacement[i]; }

private:
    static bool mUsbMidiKeyboardExists;

    int GetSlottedKeyVelocityFromExtended(int, unsigned char *);

    bool mKeyPressed[4][128]; // 0x0
    int mKeyVelocity[4][128]; // 0x200
    int mModVal[4]; // 0xa00
    int mExpressionPedal[4]; // 0xa10
    int mConnectedAccessories[4]; // 0xa20
    bool mSustain[4]; // 0xa30
    bool mStompPedal[4]; // 0xa34
    int mAccelerometer[4][3]; // 0xa38
    int mLowHandPlacement[4]; // 0xa68
    int mHighHandPlacement[4]; // 0xa78
    int mPadNum; // 0xa88
};
