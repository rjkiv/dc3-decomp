#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"

class JoypadRepeat {
public:
    JoypadRepeat();
    void Start(JoypadButton, JoypadAction, int);
    void SendRepeat(Hmx::Object *, int);
    void Poll(float, float, Hmx::Object *, int);
    void Reset(JoypadButton);

    JoypadButton mLastBtn; // 0x0
    JoypadAction mLastAction; // 0x4
    int mLastPad; // 0x8
    Timer mHoldTimer; // 0x10
    Timer mRepeatTimer; // 0x40
};

class JoypadClient : public Hmx::Object {
    friend void JoypadClientPoll();

public:
    // Hmx::Object
    virtual ~JoypadClient();
    virtual DataNode Handle(DataArray *, bool);

    JoypadClient(Hmx::Object *sink);
    void SetVirtualDpad(bool);
    void SetRepeatMask(int);

    // public: JoypadClient();
    //  public: JoypadClient(User*, Hmx::Object*);

    //  public: void SetFilterAllButStart(bool);
    //  public: bool GetFilterAllButStart();
    //  public: void SetRepeatMask(int32_t);
    //  public: int32_t GetRepeatMask() const;
    //  public: void SetHoldMs(float);
    //  public: float GetHoldMs() const;
    //  public: void SetRepeatMs(float);
    //  public: float GetRepeatMs() const;
    //  public: void SetVirtualDpad(bool);

private:
    int OnMsg(const ButtonDownMsg &);
    int OnMsg(const ButtonUpMsg &);
    void Poll();
    void Init();

    User *mUser; // 0x2c
    Hmx::Object *mSink; // 0x30
    int mBtnMask; // 0x34
    float mHoldMs; // 0x38
    float mRepeatMs; // 0x3c
    JoypadRepeat mRepeats[4]; // 0x40
    bool mVirtualDpad; // 0x200
    bool mFilterAllButStart; // 0x201
};

void JoypadClientPoll();
