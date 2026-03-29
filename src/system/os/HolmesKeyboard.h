#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/MemStream.h"

class CWnd;

class HolmesInput : public Hmx::Object {
public:
    HolmesInput(CWnd *);
    virtual ~HolmesInput();
    virtual DataNode Handle(DataArray *, bool);

    unsigned int SendJoypadMessages();
    void SendKeyboardMessages();
    void LoadKeyboard(BinStream &);
    void LoadJoypad(BinStream &);

private:
    int unk2c; // 0x2c
    MemStream *mJoypadStream; // 0x30
    MemStream *mKeyboardStream; // 0x34
    CWnd *mCWnd; // 0x38
};
