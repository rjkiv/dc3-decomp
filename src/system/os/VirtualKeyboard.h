#pragma once
#include "obj/Object.h"

class VirtualKeyboard : public Hmx::Object {
public:
    enum KeyboardConfigType {
        kKeyboard_Normal = 0,
        kKeyboard_Numeric = 1,
    };

    VirtualKeyboard();
    virtual ~VirtualKeyboard();
    virtual DataNode Handle(DataArray *, bool);

    void Init();
    void ClearKeyboardCallback();
    void Poll();
    void Terminate();
    bool IsKeyboardShowing();
    void NotifyCallbacksOpen();
    void NotifyCallbacksClose();

    void PlatformTerminate();
    DataNode OnShowKeyboardUI(const DataArray *);

private:
    Hmx::Object *mPobjKeyboardCallback; // 0x2c
    bool mCallbackReady; // 0x30
    bool mMsgOk; // 0x31
    String mCallbackMsg; // 0x34

    void PlatformPoll();
    const char *GetInputString();
    DataNode ShowKeyboardUI(
        int pad,
        int textLength,
        String strWindowTitle,
        String strDescription,
        String strDefaultText,
        int configType
    );
};

#include "obj/Msg.h"

DECLARE_MESSAGE(VirtualKeyboardResultMsg, "virtual_keyboard_result_msg");
VirtualKeyboardResultMsg(bool ok, const char *text);
bool IsOK() const { return mData->Int(2); }
const char *Text() const { return mData->Str(3); }
END_MESSAGE

extern VirtualKeyboard TheVirtualKeyboard;
