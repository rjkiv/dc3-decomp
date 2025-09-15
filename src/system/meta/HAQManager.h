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
    kHAQType_Song,
    kHAQType_Checkbox
};

class HAQManager : public Hmx::Object{
    public:
        HAQManager();
        UIComponent *GetUIFocusComponent() const;
        static void RawPrint(char const *, char const *);
        static void PrintSongInfo(Symbol, float);
        static void Print(HAQType, Hmx::Object *, int);
        static void Print(HAQType);
        virtual DataNode Handle(DataArray *, bool);
        bool Enabled() const {return m_bEnabled;}

        bool m_bEnabled;
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
};

extern HAQManager *TheHAQMgr;
