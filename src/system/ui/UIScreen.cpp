#include "ui/UIScreen.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/JoypadMsgs.h"
#include "os/Timer.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "utl/TextStream.h"

UIScreen *UIScreen::sUnloadingScreen;

UIScreen::UIScreen()
    : mFocusPanel(nullptr), mBack(nullptr), mClearVram(false), mShowing(true),
      mScreenId(sMaxScreenId++) {
    MILO_ASSERT(sMaxScreenId < 0x8000, 0x20);
}

void UIScreen::SetTypeDef(DataArray *) {}

void UIScreen::LoadPanels() {}

void UIScreen::UnloadPanels() {
    FOREACH_REVERSE(it, mPanelList) {
        if (it->mLoaded) {
            it->mPanel->CheckUnload();
        }
    }
}

bool UIScreen::CheckIsLoaded() {
    FOREACH (it, mPanelList) {
        if (it->Active() && !it->mPanel->CheckIsLoaded()) {
            return false;
        }
    }

    return true;
}

bool UIScreen::IsLoaded() const {
    FOREACH (it, mPanelList) {
        if (it->Active() && it->mPanel->GetState() == UIPanel::kUnloaded) {
            return false;
        }
    }

    // please don't tell me const_cast is what they did lol
    static Message is_loaded("is_loaded");
    DataNode result = const_cast<UIScreen *>(this)->HandleType(is_loaded);
    if (result.Type() != kDataUnhandled) {
        return result.Int();
    }

    return true;
}

void UIScreen::Poll() {
    static Message poll_msg("poll_msg");
    HandleType(poll_msg);

    FOREACH (it, mPanelList) {
        if (it->Active() && !it->mPanel->Paused()) {
            it->mPanel->Poll();
        }
    }
}

void UIScreen::Draw() {}

bool UIScreen::InComponentSelect() const {
    UIComponent *component = TheUI->FocusComponent();
    if (component != nullptr) {
        return component->GetState() == UIComponent::kSelecting;
    }

    return false;
}

void UIScreen::Enter(UIScreen *) {}

bool UIScreen::Entering() const {
    FOREACH (it, mPanelList) {
        if (it->Active() && it->mPanel->Entering()) {
            return true;
        }
    }

    if (sUnloadingScreen != nullptr && sUnloadingScreen->Unloading()) {
        return true;
    }

    sUnloadingScreen = nullptr;
    return false;
}

void UIScreen::Exit(UIScreen *to) {}

bool UIScreen::Exiting() const {
    FOREACH (it, mPanelList) {
        if (it->Active() && it->mPanel->Exiting()) {
            return true;
        }
    }

    return false;
}

void UIScreen::Print(TextStream &s) {
    static Symbol file("file");

    s << "{UIScreen " << Name() << "\n";

    if (mPanelList.size() != 0) {
        s << "   Panels:\n";
        FOREACH (it, mPanelList) {
            s << "      " << it->mPanel->Name() << " ";
            if (!it->mActive) {
                s << "(active " << it->mActive << ") ";
            }
            if (!it->mAlwaysLoad) {
                s << "(always_load " << it->mAlwaysLoad << ") ";
            }

            const DataArray *typeDef = it->mPanel->TypeDef();
            if (typeDef != nullptr) {
                DataArray *fileArray = typeDef->FindArray(file, false);
                if (fileArray != nullptr) {
                    DataNode type = fileArray->Node(1);
                    if (type.Type() == kDataString || type.Type() == kDataSymbol) {
                        s << "(" << type.LiteralStr() << ") ";
                    } else {
                        s << "(dynamic) ";
                    }
                }
            } else {
                s << " ";
            }

            if (it->mPanel == mFocusPanel) {
                s << "(focus)";
            }

            s << "\n";
        }
    }

    s << "}\n";
}

bool UIScreen::Unloading() const {
    FOREACH (it, mPanelList) {
        if (it->mLoaded && it->mPanel->Unloading()) {
            return true;
        }
    }

    return false;
}

void UIScreen::SetFocusPanel(UIPanel *panel) {
    if (panel == mFocusPanel)
        return;

    if (mFocusPanel != nullptr)
        mFocusPanel->FocusIn();

    mFocusPanel = panel;

    if (mFocusPanel != nullptr)
        mFocusPanel->FocusOut();
}

void UIScreen::SetShowing(bool show) { mShowing = show; }

bool UIScreen::HasPanel(UIPanel *panel) {
    FOREACH (it, mPanelList) {
        if (it->mPanel == panel && it->mActive) {
            return true;
        }
    }

    return false;
}

void UIScreen::ReenterScreen() {}

void UIScreen::SetPanelActive(UIPanel *panel, bool active) {
    bool found = false;
    FOREACH (it, mPanelList) {
        if (it->mPanel == panel) {
            it->mActive = active;
            found = true;
        }
    }
    MILO_ASSERT(found, 0x164);
}

bool UIScreen::AllPanelsDown() {
    FOREACH (it, mPanelList) {
        if (it->Active() && it->mPanel->GetState() != UIPanel::kDown) {
            return false;
        }
    }

    return true;
}

bool UIScreen::SharesPanels(UIScreen *screen) {
    FOREACH (it, mPanelList) {
        if (screen->HasPanel(it->mPanel)) {
            return true;
        }
    }

    return false;
}

DataNode UIScreen::OnMsg(ButtonDownMsg const &msg) {
    if (mBack != nullptr && msg.GetAction() == kAction_Cancel) {
        DataNode n = mBack->Evaluate(1);
        if (n.Type() != kDataUnhandled) {
            static Symbol go_back_screen("go_back_screen");
            Message m(go_back_screen, n.Str(), msg.GetUser());
            TheUI->Handle(m, false);
        }
    }

    return DataNode(kDataUnhandled, 0);
}

DataNode UIScreen::ForeachPanel(DataArray const *) { return NULL_OBJ; }

void UIScreen::ReloadStrings() {}

BEGIN_HANDLERS(UIScreen)

END_HANDLERS

void EnterGlitchCB(float, void *) {}

void UnloadGlitchCB(float, void *) {}
