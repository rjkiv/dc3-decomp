#include "ui/UIScreen.h"
#include "gesture/GestureMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Archive.h"
#include "os/Debug.h"
#include "os/JoypadMsgs.h"
#include "os/Timer.h"
#include "rndobj/Rnd.h"
#include "ui/UI.h"
#include "ui/UILabel.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "utl/TextStream.h"

UIScreen *UIScreen::sUnloadingScreen = nullptr;

void EnterGlitchCB(float ms, void *panel) {
    UIPanel *uiPanel = static_cast<UIPanel *>(panel);
    MILO_LOG("%s %s Enter took %.2f ms\n", uiPanel->ClassName(), uiPanel->Name(), ms);
}

void UnloadGlitchCB(float ms, void *panel) {
    UIPanel *uiPanel = static_cast<UIPanel *>(panel);
    MILO_LOG(
        "%s %s CheckUnload took %.2f ms\n", uiPanel->ClassName(), uiPanel->Name(), ms
    );
}

UIScreen::UIScreen()
    : mFocusPanel(nullptr), mBack(nullptr), mClearVram(false), mShowing(true),
      mScreenId(sMaxScreenId++) {
    MILO_ASSERT(sMaxScreenId < 0x8000, 0x20);
}

BEGIN_HANDLERS(UIScreen)
    HANDLE_EXPR(focus_panel, mFocusPanel)
    HANDLE_ACTION(set_focus_panel, SetFocusPanel(_msg->Obj<class UIPanel>(2)))
    HANDLE_ACTION(print, Print(TheDebug))
    HANDLE_ACTION(reenter_screen, ReenterScreen())
    HANDLE_ACTION(
        set_panel_active, SetPanelActive(_msg->Obj<class UIPanel>(2), _msg->Int(3))
    )
    HANDLE_ACTION(set_showing, SetShowing(_msg->Int(2)))
    HANDLE_EXPR(has_panel, HasPanel(_msg->Obj<class UIPanel>(2)))
    HANDLE_ACTION(foreach_panel, ForeachPanel(_msg))
    HANDLE_EXPR(exiting, Exiting())
    HANDLE_ACTION(reload_strings, ReloadStrings())
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_MEMBER_PTR(FocusPanel())
    HANDLE_MESSAGE(ButtonDownMsg)
END_HANDLERS

void UIScreen::SetTypeDef(DataArray *data) {
    Hmx::Object::SetTypeDef(data);
    mFocusPanel = nullptr;
    mPanelList.clear();
    static Symbol panels("panels");
    DataArray *panelsArr = data->FindArray(panels, false);
    if (panelsArr) {
        for (int i = 1; i < panelsArr->Size(); i++) {
            PanelRef pr;
            if (panelsArr->Type(i) == kDataArray) {
                static Symbol active("active");
                static Symbol always_load("always_load");
                DataArray *panelArray = panelsArr->Array(i);
                pr.mPanel = panelArray->Obj<UIPanel>(0);
                MILO_ASSERT(pr.mPanel, 0x3a);
                panelArray->FindData(active, pr.mActive, false);
                panelArray->FindData(always_load, pr.mAlwaysLoad, false);
            } else {
                pr.mPanel = panelsArr->Obj<UIPanel>(i);
                MILO_ASSERT(pr.mPanel, 0x41);
            }

            mPanelList.push_back(pr);
        }
    }
    static Symbol focus("focus");
    DataArray *focusArr = data->FindArray(focus, false);
    if (focusArr) {
        SetFocusPanel(focusArr->Obj<UIPanel>(1));
    }
    if (!mFocusPanel && !mPanelList.empty()) {
        SetFocusPanel(mPanelList.front().mPanel);
    }

    mBack = data->FindArray("back", false);
    static Symbol clear_vram("clear_vram");
    mClearVram = false;
    data->FindData(clear_vram, mClearVram, false);
}

void UIScreen::LoadPanels() {
    if (Archive::DebugArkOrder())
        MILO_LOG("ArkFile: ;%s\n", Name());

    FOREACH (it, mPanelList) {
        if (it->mAlwaysLoad || it->mPanel->IsReferenced()) {
            it->mPanel->CheckLoad();
            it->mLoaded = true;
        } else {
            it->mLoaded = false;
        }
    }
    static Message msg("load_panels");
    HandleType(msg);
}

void UIScreen::UnloadPanels() {
    FOREACH_REVERSE(it, mPanelList) {
        if (it->mLoaded) {
            AutoGlitchReport report(17.0f, UnloadGlitchCB, it->mPanel);
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
    static Message msg("is_loaded");
    DataNode result = const_cast<UIScreen *>(this)->HandleType(msg);
    if (result.Type() != kDataUnhandled) {
        return result.Int();
    }

    return true;
}

void UIScreen::Poll() {
    static Message msg("poll_msg");
    HandleType(msg);

    FOREACH (it, mPanelList) {
        if (it->Active() && !it->mPanel->Paused()) {
            it->mPanel->Poll();
        }
    }
}

void UIScreen::Draw() {
    if (mShowing) {
        FOREACH (it, mPanelList) {
            if (it->Active() && it->mPanel->Showing()
                && TheRnd.ShouldDrawPanel(it->mPanel)) {
                static Symbol suppress_blacklight_text("suppress_blacklight_text");
                const DataNode *prop = Property(suppress_blacklight_text, false);
                TheUI->SetScreenBlacklghtDisabled(prop && prop->Int() != 0);
                it->mPanel->Draw();
            }
        }
    }
}

bool UIScreen::InComponentSelect() const {
    UIComponent *component = TheUI->FocusComponent();
    if (component) {
        return component->GetState() == UIComponent::kSelecting;
    }
    return false;
}

void UIScreen::Enter(UIScreen *scr) {
    if (scr) {
        sUnloadingScreen = scr;
        scr->UnloadPanels();
    }
    Rnd::sPostProcPanelCount = 0;
    std::vector<const char *> vec;
    int i5 = 0;
    FOREACH (it, mPanelList) {
        if (it->Active() && it->mPanel->GetState() == UIPanel::kDown) {
            AutoGlitchReport report(17, EnterGlitchCB, it->mPanel);
            it->mPanel->Enter();
            if (Rnd::sPostProcPanelCount != i5) {
                vec.push_back(it->mPanel->Name());
                i5 = Rnd::sPostProcPanelCount;
            }
        }
    }
    if (Rnd::sPostProcPanelCount != 1) {
        if (Rnd::sPostProcPanelCount == 0) {
            MILO_LOG(
                "[POSTPROC WARNING] UIScreen '%s' doesn't have any panels that set the PostProc\n",
                Name()
            );
        } else {
            MILO_LOG(
                "[POSTPROC WARNING] UIScreen '%s' has %d panels that attempt to set the PostProc\n",
                Name(),
                Rnd::sPostProcPanelCount
            );
            for (int i = 0; i < Rnd::sPostProcPanelCount; i++) {
                MILO_LOG("[POSTPROC WARNING]    panel = '%s'\n", vec[i]);
            }
        }
        Rnd::sPostProcPanelCount = 0;
    }
    static Message msg("enter", 0);
    msg[0] = scr;
    HandleType(msg);
    Poll();
}

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

void UIScreen::Exit(UIScreen *to) {
    TheGestureMgr->SetInVoiceMode(false);
    static Message msg("exit", 0);
    msg[0] = to;
    HandleType(msg);

    if (to) {
        to->LoadPanels();
    }

    FOREACH (it, mPanelList) {
        if (!it->mLoaded) {
            continue;
        }

        if ((it->mPanel->ForceExit() || !to || !to->HasPanel(it->mPanel))
            && it->mPanel->GetState() == UIPanel::kUp) {
            it->mPanel->Exit();
        }
    }
}

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
            bool a = it->mActive;
            if (!a) {
                s << "(active " << a << ") ";
            }
            a = it->mAlwaysLoad;
            if (!a) {
                s << "(always_load " << a << ") ";
            }

            const DataArray *typeDef = it->mPanel->TypeDef();
            if (typeDef) {
                DataArray *fileArray = typeDef->FindArray(file, false);
                if (fileArray) {
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

void UIScreen::ReenterScreen() {
    AutoGlitchReport report(50.0f, __FUNCTION__);
    FOREACH (it, mPanelList) {
        if (it->Active()) {
            it->mPanel->Exit();
        }
    }
    FOREACH (it, mPanelList) {
        if (it->Active()) {
            it->mPanel->Enter();
        }
    }
}

void UIScreen::ReloadStrings() {
    Message msg("reload_string");
    FOREACH (it, mPanelList) {
        ObjectDir *dir = it->mPanel->DataDir();
        if (dir) {
            for (ObjDirItr<UILabel> it(dir, true); it != nullptr; ++it) {
                it->Handle(msg, true);
            }
        }
    }
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
    return DATA_UNHANDLED;
}

DataNode UIScreen::ForeachPanel(const DataArray *a) {
    DataNode *var = a->Var(2);
    DataNode n = *var;
    FOREACH (it, mPanelList) {
        if (it->mActive) {
            *var = it->mPanel;
            for (int i = 3; i < a->Size(); i++) {
                a->Command(i)->Execute();
            }
        }
    }
    *var = n;
    return 0;
}
