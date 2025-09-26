#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"

class JoypadRepeat{
    public:
        JoypadRepeat();
        void Start(JoypadButton, JoypadAction, int);
        void SendRepeat(Hmx::Object *, int);
        void Poll(float, float, Hmx::Object *, int);

        JoypadButton mLastBtn; // 0x0
        JoypadAction mLastAction; // 0x4
        int mLastPad; // 0x8
        Timer mHoldTimer; // 0x10
        Timer mRepeatTimer; // 0x40
};

class JoypadClient : public Hmx::Object{
    public:
        void SetVirtualDpad(bool);
        virtual DataNode Handle(DataArray *, bool);
        virtual ~JoypadClient();
        JoypadClient(Hmx::Object *sink);

        LocalUser *mUser; //0x2c
        Hmx::Object *mSink; //0x30
        int mBtnMask; //0x34
        float mHoldMs; //0x38
        float mRepeatMs; //0x3c
        JoypadRepeat mRepeats[4]; //0x40
        bool mVirtualDpad; //0x200
        bool mFilterAllButStart; //0x201
        
    private:
        int OnMsg(ButtonDownMsg const &);
        int OnMsg(ButtonUpMsg const &);
        void Poll();
        void Init();
};

void JoypadClientPoll();
