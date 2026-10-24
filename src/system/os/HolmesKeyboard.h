#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/MemStream.h"

class CWnd;

class HolmesInput : public Hmx::Object {
public:
    enum InputType {
        kButtonDown = 0,
        kButtonUp = 1
    };
    HolmesInput(CWnd *);
    virtual ~HolmesInput();
    virtual DataNode Handle(DataArray *, bool);

    unsigned int SendJoypadMessages();
    void SendKeyboardMessages();
    void LoadKeyboard(BinStream &);
    void LoadJoypad(BinStream &);

    // yep, this is public
    bool mSubscribe; // 0x2c
private:
    MemStream *mJoypadBuffer; // 0x30
    MemStream *mKeyboardBuffer; // 0x34
    CWnd *mOwner; // 0x38
};
