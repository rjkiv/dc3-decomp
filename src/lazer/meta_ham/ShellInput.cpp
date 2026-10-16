#include "ShellInput.h"
#include "OverlayPanel.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "game/GamePanel.h"
#include "gesture/GestureMgr.h"
#include "gesture/HandInvokeGestureFilter.h"
#include "gesture/HandsUpGestureFilter.h"
#include "gesture/SkeletonExtentTracker.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SpeechMgr.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamNavList.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/DepthBuffer.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/HelpBarPanel.h"
#include "meta_ham/LetterboxPanel.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/UIEventMgr.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/MessageTimer.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "rndobj/Anim.h"
#include "synth/Synth.h"
#include "ui/PanelDir.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

ShellInput::ShellInput()
    : mWasVoiceControlEnabled(0), mDisengageAnim(this), mDisengageFrame(0, 15, 0),
      mRightHandDisengage(0.2), mLeftHandDisengage(0.25), mLastWrongHandPosition(0),
      mWrongHandPositionAnim(this), mInputPanel(0), mCursorPanel(nullptr), mDrawCursor(0),
      mDepthBuffer(0), mSkeletonIdentifier(0), mSkeletonChooser(0),
      mSkeletonExtentTracker(0) {
    mWasHandUp[0] = false;
    mWasHandUp[1] = false;
}

ShellInput::~ShellInput() {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    handle.RemoveCallback(this);
    delete mDepthBuffer;
    delete mSkeletonIdentifier;
    delete mSkeletonChooser;
    delete mSkeletonExtentTracker;
}

BEGIN_HANDLERS(ShellInput)
    HANDLE_ACTION_IF(
        panel_navigated, !TheGestureMgr->InControllerMode(), EnterControllerMode(false)
    )
    HANDLE_EXPR(has_skeleton, HasSkeleton())
    HANDLE_EXPR(num_tracked_skeletons, NumTrackedSkeletons())
    HANDLE_ACTION(
        enter_controller_mode,
        EnterControllerMode(_msg->Size() >= 3 ? _msg->Int(2) : false)
    )
    HANDLE_ACTION(exit_controller_mode, ExitControllerMode(true))
    HANDLE_EXPR(in_controller_mode, TheGestureMgr->InControllerMode())
    HANDLE_ACTION(
        set_last_select_in_controller_mode,
        HamNavList::sLastSelectInControllerMode = _msg->Int(2)
    )
    HANDLE_EXPR(voice_control_enabled, mWasVoiceControlEnabled)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_MESSAGE(JoypadConnectionMsg)
    HANDLE_MESSAGE(SpeechRecoMessage)
    HANDLE_MESSAGE(SpeechEnableMsg)
    HANDLE_MESSAGE(LeftHandListEngagementMsg)
    HANDLE_MESSAGE(ResetControllerModeTimeoutMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void ShellInput::PostUpdate(const SkeletonUpdateData *updata) {
    if (updata) {
        Skeleton *skeleton = TheGestureMgr->GetActiveSkeleton();
        if (skeleton) {
            mInvokeOptionsFilter->Update(*skeleton, skeleton->ElapsedMs());
            mCorrectIdentityFilter->Update(*skeleton, skeleton->ElapsedMs());
        }
    }
}

void ShellInput::Init() {
    SetName("shell_input", ObjectDir::Main());
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    handle.AddCallback(this);
    mCursorPanel = ObjectDir::Main()->Find<UIPanel>("cursor_panel");
    MILO_ASSERT(mCursorPanel->CheckIsLoaded(), 95);
    MILO_ASSERT(mCursorPanel->LoadedDir(), 96);
    mDepthBuffer = new DepthBuffer();
    mDepthBuffer->Init(mCursorPanel);
    mWrongHandPositionAnim =
        mCursorPanel->DataDir()->Find<RndAnimatable>("wrong_hand_position.anim", true);
    MILO_ASSERT(TheGameData, 102);
    mSkeletonIdentifier = new SkeletonIdentifier;
    mSkeletonIdentifier->Init();
    mSkeletonChooser = new SkeletonChooser;
    mInvokeOptionsFilter = new HandInvokeGestureFilter;
    mCorrectIdentityFilter = Hmx::Object::New<HandsUpGestureFilter>();
    mCorrectIdentityFilter->SetRequiredMs(1200);
    mCursorPanel->Enter();
    TheSpeechMgr->AddSink(TheUI);
    mSkeletonExtentTracker = new SkeletonExtentTracker;

    static Symbol reset_controller_mode_timeout("reset_controller_mode_timeout");
    TheHamUI.AddSink(this, reset_controller_mode_timeout);
}

void ShellInput::Draw() { mCursorPanel->Draw(); }

void ShellInput::Poll() {
    static Symbol is_in_shell_pause("is_in_shell_pause");
    static Symbol is_in_party_mode("is_in_party_mode");
    static Symbol is_in_infinite_party_mode("is_in_infinite_party_mode");
    static bool sPracticeOptionsInvokedPartyMode = false;
    static bool sPracticeOptionsInvoked = false;
    static bool sHasSkeleton = false;

    if (TheUI->FocusPanel() == TheGamePanel) {
        if (mWrongHandPositionAnim->Frame() > 0.0f) {
            mWrongHandPositionAnim->SetFrame(0.0f, 1.0f);
            mLastWrongHandPosition = false;
        }

        if (TheGestureMgr->InControllerMode() && !TheUIEventMgr->HasActiveDialogEvent()) {
            ExitControllerMode(true);
        }
        if (!TheHamUI.InTransition()) {
            TheGestureMgr->SetIdentificationEnabled(false);
        }
        static Symbol practice("practice");
        static Symbol gameplay_mode("gameplay_mode");
        static Symbol suppress_practice_options("suppress_practice_options");
        if (TheGameMode->Property(gameplay_mode)->Sym() == practice) {
            if (mInvokeOptionsFilter->InPosition() && !sPracticeOptionsInvoked) {
                if (TheHamProvider->Property(suppress_practice_options)->Int() == 0) {
                    TheHamProvider->Export(Message("invoke_practice_options"), true);
                    sPracticeOptionsInvoked = true;
                }
            }
            if (!mInvokeOptionsFilter->InPosition()
                || TheHamProvider->Property(suppress_practice_options)->Int()) {
                if (sPracticeOptionsInvoked) {
                    TheHamProvider->Export(Message("deinvoke_practice_options"), true);
                    sPracticeOptionsInvoked = false;
                }
            }
        }
    } else if (
        TheHamProvider->Property(is_in_infinite_party_mode)->Int()
        || TheHamProvider->Property(is_in_party_mode)->Int()
    ) {
        TheGestureMgr->SetIdentificationEnabled(false);
        if (TheHamProvider->Property(is_in_shell_pause)->Int() == 0) {
            if (mInvokeOptionsFilter->InPosition() && !sPracticeOptionsInvokedPartyMode) {
                TheHamProvider->Export(Message("invoke_practice_options"), true);
                sPracticeOptionsInvokedPartyMode = true;
            }
            if (!mInvokeOptionsFilter->InPosition() && sPracticeOptionsInvokedPartyMode) {
                TheHamProvider->Export(Message("deinvoke_practice_options"), true);
                sPracticeOptionsInvokedPartyMode = false;
            }
        }
    }

    if (TheGestureMgr->InControllerMode()
        && mControllerModeTimeoutTimer.SplitMs() >= mControllerModeTimeoutDurationMs) {
        ExitControllerMode(true);
    }
    if (mCorrectIdentityFilter->HandsUp() && TheHamUI.EventDialog()
        && TheHamUI.EventDialog()->GetState() != UIPanel::kUp
        && !TheHamUI.InTransition()) {
        static Symbol ui_nav_mode("ui_nav_mode");
        static Symbol movie("movie");
        const DataNode *pNavModeNode = TheHamProvider->Property(ui_nav_mode);
        MILO_ASSERT(pNavModeNode, 0xd2);
        Symbol navmodeSym = pNavModeNode->Sym();
        if (TheGestureMgr->IDEnabled() && navmodeSym != movie) {
            mCorrectIdentityFilter->Clear();
            if (TheHamUI.GetOverlayPanel()) {
                TheHamUI.GetOverlayPanel()->Dismiss();
            }
            OverlayPanel *pCorrectPanel =
                ObjectDir::Main()->Find<OverlayPanel>("correct_identity_panel");
            MILO_ASSERT(pCorrectPanel->CheckIsLoaded(), 0xdc);
            MILO_ASSERT(pCorrectPanel->LoadedDir(), 0xdd);
            TheHamUI.SetOverlayPanel(pCorrectPanel);
        }
    }

    OverlayPanel *panel = TheHamUI.GetOverlayPanel();
    if (panel) {
        mCorrectIdentityFilter->Clear();
        if (TheHamUI.GetTransitionState() != 0) {
            panel->Dismiss();
        }
    }
    bool raised = mCorrectIdentityFilter->RaisedMs() > 0.0f;
    HamNavList::sForceDisengage = raised != false;
    mCursorPanel->Poll();
    mDepthBuffer->Poll();
    mSkeletonIdentifier->Poll();
    mSkeletonChooser->Poll();
    mSkeletonExtentTracker->Poll();

    bool hasSkel = HasSkeleton();
    if (hasSkel != sHasSkeleton) {
        static Symbol has_skeleton("has_skeleton");
        static Message updateSkeletonStatus("update_skeleton_status");
        Handle(updateSkeletonStatus, false);
        TheHamProvider->SetProperty(has_skeleton, hasSkel);
    }
    sHasSkeleton = hasSkel;

    if (TheUI->InTransition()) {
        SetCursorAlpha(0);
    }
}

void ShellInput::UpdateInputPanel(UIPanel *panel) { mInputPanel = panel; }

bool ShellInput::IsGameplayPanel() const {
    static Symbol is_gameplay_panel("is_gameplay_panel");
    if (TheUI->FocusPanel() != nullptr) {
        const DataNode *gamepanel =
            TheUI->FocusPanel()->Property(is_gameplay_panel, false);
        if (gamepanel != nullptr && gamepanel->Int() == 1)
            return true;
    }
    return false;
}

bool ShellInput::HasSkeleton() const {
    Skeleton *skel = TheGestureMgr->GetActiveSkeleton();
    return skel != nullptr && skel->IsValid();
}

int ShellInput::NumTrackedSkeletons() const {
    int count = 0;
    for (int i = 0; i < 6; i++) {
        if (TheGestureMgr->GetSkeleton(i).IsTracked())
            count++;
    }
    return count;
}

int ShellInput::CycleDrawCursor() {
    mDrawCursor = !mDrawCursor;
    return mDrawCursor;
}

void ShellInput::SyncVoiceControl() {
    static Symbol allow_voice_control("allow_voice_control");
    const DataNode *prop;
    if (mInputPanel) {
        prop = mInputPanel->Property(allow_voice_control, false);
    } else {
        prop = nullptr;
    }
    if (mInputPanel && (!prop || prop->Int() == 1) && !TheProfileMgr.GetDisableVoice()
        && !TheUIEventMgr->HasActiveDialogEvent() && TheSpeechMgr->SpeechSupported()) {
        TheSpeechMgr->SetRecognizing(true);
        mWasVoiceControlEnabled = true;

        static Symbol show_microphone_icon("show_microphone_icon");
        static Message show_microphone_msg(show_microphone_icon);
        TheHamProvider->Handle(show_microphone_msg, false);
        if (TheProfileMgr.GetShowVoiceTip()) {
            static Symbol voice_commander_help("voice_commander_help");
            static Message voice_commander_help_msg(voice_commander_help);
            TheHamProvider->Handle(voice_commander_help_msg, false);
        }
        static Symbol voice_commander_tip_temporary("voice_commander_tip_temporary");
        const DataNode *voiceProp =
            mInputPanel->Property(voice_commander_tip_temporary, false);
        if (!TheProfileMgr.GetShowVoiceTip() || (voiceProp && voiceProp->Int() == 1)) {
            TheHamProvider->SetProperty(voice_commander_tip_temporary, true);
        } else {
            TheHamProvider->SetProperty(voice_commander_tip_temporary, false);
        }
    } else {
        TheSpeechMgr->SetRecognizing(false);
        mWasVoiceControlEnabled = false;
        static Symbol hide_microphone_icon("hide_microphone_icon");
        static Message hide_microphone_msg(hide_microphone_icon);
        TheHamProvider->Handle(hide_microphone_msg, false);
        static Symbol voice_commander_help_hide("voice_commander_help_hide");
        static Message voice_commander_help_hide_msg(voice_commander_help_hide);
        TheHamProvider->Handle(voice_commander_help_hide_msg, false);
    }
}

void ShellInput::EnterControllerMode(bool b) {
    HelpBarPanel *pHelpbarPanel = TheHamUI.Helpbar();
    MILO_ASSERT(pHelpbarPanel, 0x230);
    if (pHelpbarPanel->IsControllerModeAllowed() || b) {
        pHelpbarPanel->EnterControllerMode();
        TheGestureMgr->SetInControllerMode(true);
        static Message controller_mode_entered("controller_mode_entered");
        TheUI->Handle(controller_mode_entered, false);
        mLastWrongHandPosition = false;
        static Symbol in_controller_mode("in_controller_mode");
        TheHamProvider->SetProperty(in_controller_mode, true);
        TheRockCentral.EnterControllerMode();
        mControllerModeTimeoutTimer.Restart();
        int hamUIPadNum = TheHamUI.GetLastButtonDownPadNum();
        if (!TheProfileMgr.GetCriticalProfile()) {
            for (int i = 0; i < 2; i++) {
                HamPlayerData *pPlayer = TheGameData->Player(i);
                MILO_ASSERT(pPlayer, 0x251);
                if (pPlayer->PadNum() == hamUIPadNum) {
                    mSkeletonChooser->SetActivePlayer(i);
                    return;
                }
            }
        }
    } else {
        TheSynth->RunFlow("invalid_select.flow");
    }
}

void ShellInput::ExitControllerMode(bool b) {
    if (TheHamUI.Helpbar())
        TheHamUI.Helpbar()->ExitControllerMode(b);
    TheGestureMgr->SetInControllerMode(false);
    static Message controllerModeExited("controller_mode_exited");
    TheUI->Handle(controllerModeExited, false);
    static Symbol in_controller_mode("in_controller_mode");
    TheHamProvider->SetProperty(in_controller_mode, 0);
    TheRockCentral.ExitControllerMode();
}

void ShellInput::DrawDebug() {
    if (mInputPanel) {
        HamNavList *list =
            mInputPanel->DataDir()->Find<HamNavList>("right_hand.hnl", false);
        if (list) {
            list->DrawDebug();
        }
    }
    mSkeletonIdentifier->DrawDebug();
    mSkeletonChooser->DrawDebug();
}

void ShellInput::SetCursorAlpha(float f1) const {
    if (TheHamUI.Helpbar()) {
        ObjectDir *hbpDataDir = TheHamUI.Helpbar()->DataDir();
        PanelDir *dir = dynamic_cast<PanelDir *>(hbpDataDir);
        if (dir) {
            RndAnimatable *anim =
                dir->DataDir()->Find<RndAnimatable>("cursor_alpha.anim");
            float frame = anim->Frame();
            frame = TheTaskMgr.DeltaUISeconds() * (f1 - frame) * 10.0f + frame;
            anim->SetFrame(frame, 1);
        }
    }
}

void ShellInput::SyncToCurrentScreen() {
    if (TheUIEventMgr->HasActiveDialogEvent()
        && TheHamUI.EventDialog()->GetState() == UIPanel::kUp) {
        mInputPanel = TheHamUI.EventDialog();
    } else if (TheHamUI.GetOverlayPanel()) {
        mInputPanel = TheHamUI.GetOverlayPanel();
    } else {
        mInputPanel = TheHamUI.FocusPanel();
    }
    TheGestureMgr->SetUIMode(!IsGameplayPanel());
    TheHamUI.Helpbar()->SyncToPanel(mInputPanel);
    mControllerModeTimeoutDurationMs = 5000;
    if (TheHamUI.Helpbar()) {
        static Symbol controller_mode_timeout("controller_mode_timeout");
        const DataNode *prop =
            TheHamUI.Helpbar()->Property(controller_mode_timeout, false);
        if (prop) {
            mControllerModeTimeoutDurationMs = prop->Int();
        }
    }
    LetterboxPanel *lbp = TheHamUI.Letterbox();
    if (lbp) {
        lbp->SyncToPanel(mInputPanel);
    }
    static Symbol use_gamertag_bg("use_gamertag_bg");
    bool use = false;
    if (mInputPanel) {
        SyncVoiceControl();
        const DataNode *prop = mInputPanel->Property(use_gamertag_bg, false);

        if (prop) {
            use = prop->Int();
        }
        static Symbol allow_doubleuser_swipe("allow_doubleuser_swipe");
        const DataNode *userProp = mInputPanel->Property(allow_doubleuser_swipe, false);
        if (userProp && userProp->Int() == 1) {
            TheGestureMgr->SetInDoubleUserMode(true);
        } else {
            TheGestureMgr->SetInDoubleUserMode(false);
        }
    }
    TheHamProvider->SetProperty(use_gamertag_bg, use);
}

DataNode ShellInput::OnMsg(const SpeechEnableMsg &msg) {
    if (msg->Int(2))
        SyncVoiceControl();
    return 0;
}

DataNode ShellInput::OnMsg(const ResetControllerModeTimeoutMsg &msg) {
    mControllerModeTimeoutTimer.Restart();
    return DATA_UNHANDLED;
}

DataNode ShellInput::OnMsg(const ButtonDownMsg &msg) {
    if (msg.GetButton() == kPad_RStickUp || msg.GetButton() == kPad_RStickDown
        || msg.GetButton() == kPad_RStickLeft || msg.GetButton() == kPad_RStickRight
        || msg.GetButton() == kPad_Xbox_LT || msg.GetButton() == kPad_Xbox_RT) {
        TheSynth->RunFlow("invalid_select.flow");
        return DATA_UNHANDLED;
    } else if (TheGestureMgr->InControllerMode() && msg.GetButton() == kPad_Start) {
        ExitControllerMode(false);
        return 0;
    } else if (mInputPanel) {
        bool b2 = false;
        static Symbol is_gameplay_panel("is_gameplay_panel");
        const DataNode *prop = mInputPanel->Property(is_gameplay_panel, false);
        if (prop && prop->Int() == 1) {
            b2 = true;
        }
        if (!b2 && !TheGestureMgr->InControllerMode()) {
            EnterControllerMode(false);
            if (msg.GetButton() != kPad_Xbox_B) {
                if (!TheHamUI.Helpbar()->IsControllerModeAllowed()) {
                    TheSynth->RunFlow("invalid_select.flow");
                }
                return 0;
            }
        }
        return DATA_UNHANDLED;
    } else {
        return DATA_UNHANDLED;
    }
}

DataNode ShellInput::OnMsg(const JoypadConnectionMsg &msg) {
    if (TheGestureMgr->InControllerMode() && !msg.Connected()) {
        if (msg->Int(5) == TheHamUI.GetLastButtonDownPadNum())
            ExitControllerMode(false);
    }
    return DATA_UNHANDLED;
}

DataNode ShellInput::OnMsg(const SpeechRecoMessage &msg) { return 0; }

DataNode ShellInput::OnMsg(const LeftHandListEngagementMsg &msg) {
    if (msg.Engaged()) {
        static Symbol voice_commander_help_hide("voice_commander_help_hide");
        static Message voice_commander_help_hide_msg(voice_commander_help_hide);
        TheHamProvider->Handle(voice_commander_help_hide_msg, false);

    } else {
        if (!mInputPanel) {
            return DataNode(kDataInt, 0);
        }
        static Symbol allow_voice_control("allow_voice_control");
        const DataNode *voiceProp = mInputPanel->Property(allow_voice_control, false);
        if (!voiceProp || voiceProp->Int() != 1
            || TheProfileMgr.GetDisableVoiceCommander()
            || !TheProfileMgr.GetShowVoiceTip()) {
            return DataNode(kDataInt, 0);
        }
        static Symbol voice_commander_tip_temporary("voice_commander_tip_temporary");
        const DataNode *voiceHelpProp =
            mInputPanel->Property(voice_commander_tip_temporary, false);
        if (voiceHelpProp && voiceHelpProp->Int() != 0) {
            return DataNode(kDataInt, 0);
        }
        static Symbol voice_commander_help("voice_commander_help");
        static Message voice_commander_help_msg(voice_commander_help);
        TheHamProvider->Handle(voice_commander_help_msg, false);
    }
    return DataNode(kDataInt, 0);
}
