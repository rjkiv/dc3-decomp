#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Joypad.h"
#include "ui/UIComponent.h"
#include "ui/UIList.h"
#include "ui/UISlider.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

class UIComponent;
class UIList;
class UISlider;

enum HAQType {
    kHAQType_Screen,
    kHAQType_Focus,
    kHAQType_Button,
    kHAQType_Slider,
    kHAQType_List,
    kHAQType_Song
};

class HAQManager : public Hmx::Object {
public:
    HAQManager();
    // Hmx::Object
    virtual ~HAQManager();
    virtual DataNode Handle(DataArray *, bool);

    UIComponent *GetUIFocusComponent() const;
    static void RawPrint(char const *, char const *);
    static void PrintSongInfo(Symbol, float);
    static void Print(HAQType, Hmx::Object *, int);
    static void Print(HAQType);
    bool Enabled() const { return m_bEnabled; }

private:
    String GetLabelForType(HAQType) const;
    String GetScreenText() const;
    String GetFocusText() const;
    String GetPressedStringForButton(int, JoypadButton, String) const;
    String GetButtonStatePressedString(int) const;
    String GetButtonText() const;
    String GetTextForType(HAQType) const;
    void PrintList(UIList *);
    void PrintSlider(UISlider *);
    void PrintComponentInfo(UIComponent *);
    void ToggleEnabled();
    void DisplayAll();

    bool m_bEnabled; // 0x2c
};

extern HAQManager *TheHAQMgr;
