#include "meta_ham/LetterboxPanel.h"
#include "LetterboxPanel.h"
#include "flow/Flow.h"
#include "flow/FlowNode.h"
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "rndobj/Group.h"
#include "rndobj/PropAnim.h"
#include "rndobj/Tex.h"
#include "rndobj/Text.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

LetterboxPanel::LetterboxPanel()
    : unk3c(0), unk40(0), mIsBlacklightMode(false), unk45(false), unk7c(0), unk78(13000) {
    sInstance = this;
}

LetterboxPanel::~LetterboxPanel() { sInstance = nullptr; }

BEGIN_PROPSYNCS(LetterboxPanel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void LetterboxPanel::Unload() { UIPanel::Unload(); }

void LetterboxPanel::Enter() {
    unk40 = DataDir()->Find<RndGroup>("letterbox_main.grp");
    static Symbol blacklight_timeout("blacklight_timeout");
    const DataNode *property = Property(blacklight_timeout, false);
    if (property) {
        unk78 = property->Int();
    }
    HamPanel::Enter();
}

void LetterboxPanel::Draw() {
    if (unk40) {
        unk40->SetShowing(!ShouldHideLetterbox());
    }
    UIPanel::Draw();
}

void LetterboxPanel::Poll() {
    HamPanel::Poll();
    if (!ShouldHideLetterbox() && mIsBlacklightMode && unk7c != 0) {
        if (unk7c == 1) {
            unk48.Restart();
            unk7c = 2;
            unk80 = unk78;
        } else {
            if (unk48.SplitMs() >= unk80) {
                unk48.Restart();
                if (unk7c != 2) {
                    if (unk7c == 6)
                        unk7c = 0;
                } else {
                    unk7c = 6;
                    unk80 = 100.0f;
                    SetBlacklightMode(false);
                }
            }
        }
    }
}

bool LetterboxPanel::IsBlacklightMode() { return mIsBlacklightMode; }

bool LetterboxPanel::IsLeavingBlacklightMode() {
    Flow *f = DataDir()->Find<Flow>("deactivate_letterbox.flow", false);
    return f && f->IsRunning();
}

bool LetterboxPanel::IsEnteringBlacklightMode() {
    Flow *f = DataDir()->Find<Flow>("activate_letterbox.flow", false);
    return f && f->IsRunning();
}

bool LetterboxPanel::InBlacklightTransition() {
    return IsEnteringBlacklightMode() || IsLeavingBlacklightMode();
}

void LetterboxPanel::VoiceInput(int i, bool b) {
    if (mIsBlacklightMode && b) {
        Flow *f;
        if (i == 0) {
            f = DataDir()->Find<Flow>("voice_command_right.flow", false);
        } else {
            f = DataDir()->Find<Flow>("voice_command_left.flow", false);
        }
        if (f) {
            f->Activate();
        }
    }
}

void LetterboxPanel::EnterBlacklightMode() {
    mIsBlacklightMode = true;
    Flow *f = DataDir()->Find<Flow>("activate_letterbox.flow", false);
    if (f)
        f->Activate();
    if (unk45 != true) // wont let me do !unk45
        unk45 = true;
    RndText::SetBlacklightModeEnabled(true);
    static Message enter_blacklight_mode("enter_blacklight_mode");
    Handle(enter_blacklight_mode, false);
}

void LetterboxPanel::ExitBlacklightMode(bool b) {
    mIsBlacklightMode = false;
    Flow *f;
    if (!b) {
        f = DataDir()->Find<Flow>("deactivate_letterbox.flow", false);
    } else {
        f = DataDir()->Find<Flow>("deactivate_letterbox_immediate.flow", false);
    }
    if (f)
        f->Activate();
    if (unk45 != false)
        unk45 = false;
    DataNode handle;
    RndText::SetBlacklightModeEnabled(false);
    if (b) {
        static Message exit_blacklight_mode("exit_blacklight_mode");
        handle = Handle(exit_blacklight_mode, false);
    } else {
        static Message exit_blacklight_mode("exit_blacklight_mode");
        handle = Handle(exit_blacklight_mode, false);
    }
}

bool LetterboxPanel::ShouldHideLetterbox() const {
    static Symbol hide_letterbox("hide_letterbox");
    if (unk3c && Property(hide_letterbox, false)->Int())
        return true;
    return false;
}

bool LetterboxPanel::ShouldShowHandHelp() const {
    static Symbol show_letterbox_hand_help("show_letterbox_hand_help");
    if (unk3c && Property(show_letterbox_hand_help, false)->Int()) {
        return true;
    }
    return false;
}

void LetterboxPanel::ToggleBlacklightMode(bool toggle) {
    if (toggle) {
        SetBlacklightModeImmediately(mIsBlacklightMode == false);
        return;
    }
    SetBlacklightMode(mIsBlacklightMode == false);
}

void LetterboxPanel::SetBlacklightMode(bool b) {
    if (b != mIsBlacklightMode) {
        if (!ShouldHideLetterbox() || !b) {
            if (!InBlacklightTransition()) {
                HandleType(Message("on_toggle_blacklight", b));
                mIsBlacklightMode = b;
                if (b) {
                    EnterBlacklightMode();
                    unk7c = 1;
                    unk48.Restart();
                    int temp = unk78;
                    unk80 = temp;
                } else
                    ExitBlacklightMode(false);
            }
        }
    }
    if (unk7c == 2) {
        unk48.Restart();
        unk80 = unk78;
    }
}

void LetterboxPanel::SetBlacklightModeImmediately(bool b) {
    if (b != mIsBlacklightMode) {
        if (!ShouldHideLetterbox() || !b) {
            HandleType(Message("on_toggle_blacklight", b));
            mIsBlacklightMode = b;
            if (b) {
                EnterBlacklightMode();
                unk7c = 1;
                unk48.Restart();
                int temp = unk78;
                unk80 = temp;
            } else
                ExitBlacklightMode(true);
        }
    }
    if (unk7c == 2) {
        unk48.Restart();
        unk80 = unk78;
    }
}

BEGIN_HANDLERS(LetterboxPanel)
    HANDLE_ACTION(resync, SyncToPanel(unk3c))
    HANDLE_ACTION(sync_to_panel, SyncToPanel(_msg->Obj<UIPanel>(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
