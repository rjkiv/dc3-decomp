#include "ui/UI.h"
#include "UIComponent.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/MessageTimer.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Joypad.h"
#include "os/JoypadClient.h"
#include "os/JoypadMsgs.h"
#include "os/Keyboard.h"
#include "os/System.h"
#include "os/UserMgr.h"
#include "rndobj/Cam.h"
#include "ui/CheatProvider.h"
#include "ui/InlineHelp.h"
#include "ui/LabelNumberTicker.h"
#include "ui/LabelShrinkWrapper.h"
#include "ui/LocalePanel.h"
#include "ui/PanelDir.h"
#include "ui/Screenshot.h"
#include "ui/UIButton.h"
#include "ui/UIColor.h"
#include "ui/UIFontImporter.h"
#include "ui/UIGuide.h"
#include "ui/UILabel.h"
#include "ui/UIList.h"
#include "ui/UIPicture.h"
#include "ui/UIScreen.h"
#include "ui/UIPanel.h"
#include "ui/UISlider.h"
#include "ui/UITrigger.h"
#include "utl/Cheats.h"
#include "utl/FilePath.h"
#include "utl/KnownIssues.h"
#include "utl/Locale.h"
#include "utl/OSCMessenger.h"
#include "utl/Std.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

namespace {
    JoypadAction NavButtonToNavAction(JoypadButton btn) {
        switch (btn) {
        case kPad_DLeft:
            return kAction_Left;
        case kPad_DRight:
            return kAction_Right;
        case kPad_DDown:
            return kAction_Down;
        case kPad_DUp:
            return kAction_Up;
        default:
            return kAction_None;
        }
    }
}

const char *TransitionStateString(UIManager::TransitionState s) {
    switch (s) {
    case UIManager::kTransitionTo:
        return "to";
    case UIManager::kTransitionFrom:
        return "from";
    case UIManager::kTransitionPop:
        return "pop";
    default:
        return "";
    }
}

void TerminateCallback() {
    MILO_ASSERT(TheUI, 0x1CE);
    TheUI->Terminate();
}

void FailAppendCallback(FixedString &str) {
    if ((TheUI && TheUI->CurrentScreen()) || TheUI->TransitionScreen()) {
        str += "\n";
        if (TheUI->CurrentScreen()) {
            str += "Screen: ";
            str += TheUI->CurrentScreen()->Name();
        }
        if (TheUI->InTransition()) {
            str += "Trans: ";
            str += TransitionStateString(TheUI->GetTransitionState());
            str += " ";
            str += TheUI->TransitionScreen()->Name();
        }
    }
}

void UITerminateCallback() { TheUI->Terminate(); }

#pragma region UIManager

UIManager::UIManager()
    : mWentBack(0), mMaxPushDepth(100), mJoyClient(0), mCurrentScreen(0), mSink(0),
      mOverloadHorizontalNav(0), mCancelTransitionNotify(0), mDefaultAllowEditText(1),
      mDisableScreenBlacklight(0), mOverlay(0), mAutomator(0), mShowDevMenu(0) {}

UIManager::~UIManager() {}

BEGIN_HANDLERS(UIManager)
    if ((InTransition() || InComponentSelect())
        && BlockHandlerDuringTransition(sym, _msg)) {
        return 0;
    }
    HANDLE_MEMBER_PTR(mSink)
    HANDLE_ACTION(set_sink, mSink = _msg->Obj<Hmx::Object>(2))
    HANDLE_ACTION(use_joypad, UseJoypad(_msg->Int(2), true))
    HANDLE_ACTION(set_virtual_dpad, mJoyClient->SetVirtualDpad(_msg->Int(2)))
    HANDLE_ACTION(push_screen, PushScreen(_msg->Obj<UIScreen>(2)))
    HANDLE_ACTION_IF_ELSE(
        pop_screen, _msg->Size() > 2, PopScreen(_msg->Obj<UIScreen>(2)), PopScreen(0)
    )
    HANDLE(goto_screen, OnGotoScreen)
    HANDLE(go_back_screen, OnGoBackScreen)
    HANDLE_ACTION(reset_screen, ResetScreen(_msg->Obj<UIScreen>(2)))
    HANDLE_EXPR(focus_panel, FocusPanel())
    HANDLE_EXPR(current_screen, CurrentScreen())
    HANDLE_EXPR(transition_screen, TransitionScreen())
    HANDLE_EXPR(bottom_screen, BottomScreen())
    HANDLE_EXPR(in_transition, InTransition())
    HANDLE(is_resource, OnIsResource)
    HANDLE(foreach_current_screen, OnForeachCurrentScreen)
    HANDLE_EXPR(went_back, WentBack())
    HANDLE_EXPR(is_game_screen_active, IsGameScreenActive())
    HANDLE_ACTION(toggle_load_times, ToggleLoadTimes())
    HANDLE_EXPR(showing_load_times, mOverlay->Showing())
    HANDLE_ACTION(toggle_dev_menu, mShowDevMenu = !mShowDevMenu)
    HANDLE_EXPR(show_dev_menu, mShowDevMenu)
    HANDLE_MEMBER_PTR(mAutomator)
    HANDLE_ACTION(
        fake_keyboard_action,
        FakeKeyboardAction((JoypadButton)_msg->Int(2), (JoypadAction)_msg->Int(3))
    )
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_MEMBER_PTR(mCurrentScreen)
END_HANDLERS

void UIManager::Init() {
    MILO_ASSERT(TheUI, 0x1f3);
    mAutomator = new Automator(*this);
    SetName("ui", ObjectDir::Main());
    DataArray *cfg = SystemConfig("ui");
    SetTypeDef(SystemConfig("ui"));
    UseJoypad(cfg->FindInt("use_joypad"), cfg->FindInt("enable_auto_repeat"));
    KeyboardSubscribe(this);
    mCurrentScreen = nullptr;
    mTransitionState = kTransitionNone;
    mTransitionScreen = nullptr;
    mWentBack = false;
    mCam = ObjectDir::Main()->New<RndCam>("[ui.cam]");
    DataArray *camCfg = cfg->FindArray("cam");
    mCam->SetFrustum(
        camCfg->FindFloat("near"),
        camCfg->FindFloat("far"),
        camCfg->FindFloat("fov") * DEG2RAD,
        1.0f
    );
    mCam->SetLocalPos(Vector3(0, camCfg->FindFloat("y"), 0));
    DataArray *zArr = camCfg->FindArray("z-range");
    mCam->SetZRange(zArr->Float(1), zArr->Float(2));
    mEnv = Hmx::Object::New<RndEnviron>();
    Hmx::Color envAmbientColor;
    cfg->FindArray("env")->FindData("ambient", envAmbientColor, true);
    mEnv->SetAmbientColor(envAmbientColor);
    cfg->FindData("max_push_depth", mMaxPushDepth, false);
    cfg->FindData("cancel_transition_notify", mCancelTransitionNotify, false);
    cfg->FindData("default_allow_edit_text", mDefaultAllowEditText, false);
    bool notify = false;
    cfg->FindData("verbose_locale_notifies", notify, false);
    Locale::SetLocaleVerboseNotify(notify);
    REGISTER_OBJ_FACTORY(UIScreen)
    REGISTER_OBJ_FACTORY(UIPanel)
    REGISTER_OBJ_FACTORY(PanelDir)
    UIComponent::Init();
    UIButton::Init();
    REGISTER_OBJ_FACTORY(UIColor)
    UILabel::Init();
    UIList::Init();
    REGISTER_OBJ_FACTORY(UIPicture)
    UISlider::Init();
    REGISTER_OBJ_FACTORY(UITrigger)
    InlineHelp::Init();
    REGISTER_OBJ_FACTORY(UIFontImporter)
    REGISTER_OBJ_FACTORY(UIGuide)
    REGISTER_OBJ_FACTORY(Screenshot)
    LabelNumberTicker::Init();
    LabelShrinkWrapper::Init();
    TheDebug.AddExitCallback(TerminateCallback);

    std::vector<ObjDirPtr<ObjectDir> > dirPtrs;
    DataArray *frontloadArr = cfg->FindArray("frontload_subdirs", false);
    if (frontloadArr) {
        dirPtrs.resize(frontloadArr->Size() - 1);
        for (int i = 1; i < frontloadArr->Size(); i++) {
            String curStr = frontloadArr->Str(i);
            dirPtrs[i - 1].LoadFile(curStr.c_str(), false, true, kLoadFront, false);
        }
    }
    CheatProvider::Init();
    REGISTER_OBJ_FACTORY(LocalePanel)
    static Message cheat_init("cheat_init");
    Hmx::Object::Handle(cheat_init, false);
    mOverlay = RndOverlay::Find("ui", true);
    mOverlay->SetShowing(false);
    TheOSCMessenger.Connect();
    TheDebug.AddFailAppendCallback(FailAppendCallback);
    PreloadSharedSubdirs("ui");
    UILabel::sRequireFixedLength = true;
    static Message init("init");
    Hmx::Object::Handle(init, false);
    UILabel::sRequireFixedLength = false;
    cfg->FindData("overload_horizontal_nav", mOverloadHorizontalNav, false);
    TheKnownIssues.Init();
}

void UIManager::Terminate() {
    CheatProvider::Terminate();
    UILabel::Terminate();
    SetName(0, 0);
    KeyboardUnsubscribe(this);
    RELEASE(mCam);
    RELEASE(mEnv);
    RELEASE(mJoyClient);
    TheDebug.RemoveExitCallback(TerminateCallback);
    RELEASE(mAutomator);
}

void UIManager::Draw() {
    FOREACH (it, mPushedScreens) {
        (*it)->Draw();
    }
    if (mCurrentScreen)
        mCurrentScreen->Draw();
}

void UIManager::GotoScreen(const char *name, bool b2, bool b3) {
    UIScreen *screen = ObjectDir::Main()->Find<UIScreen>(name, true);
    MILO_ASSERT(screen, 0x37E);
    GotoScreen(screen, b2, b3);
}

void UIManager::GotoScreen(UIScreen *scr, bool b1, bool b2) {
    GotoScreenImpl(scr, b1, b2);
}

void UIManager::PushScreen(UIScreen *screen) {
    MILO_ASSERT(screen, 0x38C);
    if (!mCurrentScreen) {
        MILO_NOTIFY(
            "Called PushScreen() with %s when mCurrentScreen is NULL, are you calling PushScreen() twice in the same frame?",
            screen->Name()
        );
    } else {
        CancelTransition();
        if (mCurrentScreen) {
            mPushedScreens.push_back(mCurrentScreen);
        } else {
            MILO_LOG("UIManager::PushScreen NULL current screen. Not pushing it.\n");
        }
        if (mPushedScreens.size() >= mMaxPushDepth) {
            MILO_NOTIFY(
                "Exceeded max push depth of %i, pushing %s", mMaxPushDepth, screen->Name()
            );
            MILO_LOG("mPushedScreens:\n");
            FOREACH (it, mPushedScreens) {
                if (*it) {
                    MILO_LOG("%s\n", (*it)->Name());
                } else {
                    MILO_LOG("NULL pushed screen? That's pretty bad.\n");
                }
            }
        }
        mCurrentScreen = nullptr;
        GotoScreenImpl(screen, false, false);
    }
}

void UIManager::PopScreen(UIScreen *screen) {
    if (mPushedScreens.empty()) {
        MILO_NOTIFY("No screen to pop\n");
    } else {
        GotoScreenImpl(nullptr, false, false);
        mTransitionState = kTransitionPop;
        if (screen)
            mTransitionScreen = screen;
        else
            mTransitionScreen = mPushedScreens.back();
    }
}

void UIManager::ResetScreen(UIScreen *screen) {
    if (mTransitionState != kTransitionNone && mTransitionState != kTransitionFrom) {
        bool old = mCancelTransitionNotify;
        mCancelTransitionNotify = false;
        CancelTransition();
        mCancelTransitionNotify = old;
    }
    if (mPushedScreens.empty()) {
        GotoScreen(screen, false, false);
    } else {
        MILO_ASSERT(mPushedScreens.size() == 1, 0x3E5);
        PopScreen(screen);
    }
}

bool UIManager::InComponentSelect() {
    if (mCurrentScreen)
        return mCurrentScreen->InComponentSelect();
    else
        return false;
}

void UIManager::SetScreenBlacklghtDisabled(bool disable) {
    mDisableScreenBlacklight = disable;
}

UIPanel *UIManager::FocusPanel() {
    if (mCurrentScreen)
        return mCurrentScreen->FocusPanel();
    else
        return nullptr;
}

UIComponent *UIManager::FocusComponent() {
    UIPanel *focusPanel = FocusPanel();
    if (focusPanel)
        return focusPanel->FocusComponent();
    else
        return nullptr;
}

void UIManager::ToggleLoadTimes() {
    mOverlay->CurrentLine() = gNullStr;
    mOverlay->SetShowing(!mOverlay->Showing());
}

void UIManager::GotoFirstScreen() {
    GotoScreen(DataVariable("first_screen").Obj<UIScreen>(), false, false);
    mTimer.Restart();
}

void UIManager::FakeKeyboardAction(JoypadButton btn, JoypadAction act) {
    static ButtonDownMsg downMsg(nullptr, kPad_NumButtons, kAction_None, 0);
    downMsg[0] = TheUserMgr->GetLocalUserFromPadNum(0);
    downMsg[1] = btn;
    downMsg[2] = act;
    downMsg[3] = 0;
    Handle(downMsg, false);
}

UIScreen *UIManager::BottomScreen() {
    return !mPushedScreens.empty() ? mPushedScreens.front() : mCurrentScreen;
}

int UIManager::PushDepth() const { return mPushedScreens.size(); }

UIScreen *UIManager::ScreenAtDepth(int depth) {
    MILO_ASSERT(depth < mPushedScreens.size(), 0x46F);
    return mPushedScreens[depth];
}

void UIManager::UseJoypad(bool useJoypad, bool enableAutoRepeat) {
    if (useJoypad && !mJoyClient) {
        mJoyClient = new JoypadClient(this);
        mJoyClient->SetVirtualDpad(true);
        if (enableAutoRepeat) {
            mJoyClient->SetRepeatMask(0xf000);
        }
    } else if (!useJoypad) {
        if (mJoyClient) {
            RELEASE(mJoyClient);
        }
    }
}

void UIManager::CancelTransition() {
    if (mCancelTransitionNotify && mTransitionState != kTransitionNone
        && mTransitionState != kTransitionFrom) {
        MILO_NOTIFY("Cancelled transition");
    }
    TransitionState oldState = mTransitionState;
    UIScreen *oldScreen = mTransitionScreen;
    mTransitionState = kTransitionNone;
    mTransitionScreen = nullptr;
    if (oldState == kTransitionTo) {
        if (mCurrentScreen) {
            mCurrentScreen->Enter(oldScreen);
        } else if (oldScreen)
            oldScreen->UnloadPanels();
    } else if (oldState == kTransitionPop && mCurrentScreen) {
        mCurrentScreen->Enter(nullptr);
    }
}

void UIManager::ReloadStrings() {
    Message msg("reload_strings");
    if (mCurrentScreen) {
        mCurrentScreen->Handle(msg, true);
    }
    FOREACH (it, mPushedScreens) {
        (*it)->Handle(msg, true);
    }
}

bool UIManager::BlockHandlerDuringTransition(Symbol s, DataArray *a) {
    if (s == KeyboardKeyMsg::Type()) {
        return true;
    } else if (s == ButtonDownMsg::Type() || s == ButtonUpMsg::Type()) {
        UIPanel *focus = FocusPanel();
        if (focus) {
            static Symbol allowed_transition_actions("allowed_transition_actions");
            const DataNode *prop = focus->Property(allowed_transition_actions, false);
            DataArray *val = prop ? prop->Array() : nullptr;
            if (val) {
                for (int i = 0; i < val->Size(); i++) {
                    if (val->Int(i) == a->Int(4)) {
                        return false;
                    }
                }
            }
        }
        return true;
    } else {
        return false;
    }
}

bool UIManager::OverloadHorizontalNav(JoypadAction act, JoypadButton btn, bool b) const {
    return !(!mOverloadHorizontalNav || NavButtonToNavAction(btn) == act && !b);
}

void UIManager::GotoScreenImpl(UIScreen *scr, bool b1, bool b2) {
    if (b1 || mTransitionState != kTransitionNone
        || mCurrentScreen != scr
            && (mTransitionState != kTransitionTo && mTransitionState != kTransitionPop)
        || mTransitionScreen != scr) {
        CancelTransition();

        if (scr) {
            FOREACH (it, mPushedScreens) {
                if (scr->SharesPanels(*it)) {
                    MILO_FAIL("%s shares panels with %s", scr->Name(), (*it)->Name());
                }
            }
        }

        mWentBack = b2;
        // UIScreenChangeMsg msg(scr, mCurrentScreen, mWentBack);
        // Handle(msg, false);
        mTransitionState = kTransitionTo;
        mTransitionScreen = scr;
        if (mCurrentScreen)
            mCurrentScreen->Exit(scr);
        else if (scr)
            scr->LoadPanels();

        if (mTransitionScreen) {
            mOverlay->CurrentLine() = gNullStr;
            mLoadTimer.Restart();
        }
    }
}

bool UIManager::IsGameScreenActive() {
    bool ret = BottomScreen() && streq(BottomScreen()->Name(), "game_screen");
    ret &= mCurrentScreen != BottomScreen();
    return ret;
}

DataNode UIManager::OnIsResource(DataArray *a) {
    Symbol sym = a->Sym(3);
    static Symbol objects("objects");
    static Symbol resources_path("resources_path");
    DataArray *symArr = SystemConfig(objects, sym)->FindArray(resources_path, false);
    if (symArr) {
        FilePath fp1(FileMakePath(FileGetPath(symArr->File()), symArr->Str(1)));
        FilePath fp2(FileRoot(), a->Str(2));
        if (fp1 == FileGetPath(fp2.c_str())) {
            return 1;
        }
    } else {
        MILO_NOTIFY("%s does not have a resources_path set", sym);
    }
    return 0;
}

DataNode UIManager::OnGotoScreen(DataArray const *arr) {
    Hmx::Object *obj = arr->GetObj(2);
    UIScreen *screen = dynamic_cast<UIScreen *>(obj);
    if (screen == nullptr && obj)
        MILO_FAIL("%s is not a screen", obj->Name());

    if (arr->Size() > 4) {
        GotoScreen(screen, arr->Int(3), arr->Int(4));
    } else if (arr->Size() > 3) {
        GotoScreen(screen, arr->Int(3), false);
    } else {
        GotoScreen(screen, false, false);
    }
    return 0;
}

DataNode UIManager::OnGoBackScreen(DataArray const *arr) {
    Hmx::Object *obj = arr->GetObj(2);
    UIScreen *screen = dynamic_cast<UIScreen *>(obj);
    if (screen == nullptr && obj) {
        MILO_FAIL("%s is not a screen", obj->Name());
    }
    GotoScreen(screen, false, true);
    return DATA_UNHANDLED;
}

DataNode UIManager::OnForeachCurrentScreen(const DataArray *arr) {
    DataNode *var = arr->Var(2);
    DataNode n(*var);
    std::vector<UIScreen *> screens(mPushedScreens);
    if (mCurrentScreen) {
        screens.push_back(mCurrentScreen);
    }
    FOREACH (it, screens) {
        *var = *it;
        for (int i = 3; i < arr->Size(); i++) {
            arr->Command(i)->Execute();
        }
    }
    *var = n;
    return 0;
}

bool UIManager::DefaultAllowEditText() const { return mDefaultAllowEditText; }

#pragma endregion
#pragma region Automator

Automator::Automator(UIManager &mgr)
    : mUIManager(mgr), mScreenScripts(0), mRecord(0), mAutoPath("automator.dta"),
      mRecordPath("automator.dta"), mCurScript(0), mSkipNextQuickCheat(0) {}

Automator::~Automator() {
    if (mScreenScripts) {
        mScreenScripts->Release();
        mScreenScripts = 0;
    }
    FinishRecord();
}

BEGIN_HANDLERS(Automator)
    HANDLE_EXPR(toggle_auto, ToggleAuto())
    HANDLE_EXPR(auto_script, AutoScript())
    HANDLE_EXPR(toggle_record, ToggleRecord())
    HANDLE_EXPR(record_script, mRecord ? mRecordPath.c_str() : "OFF")
    HANDLE_ACTION(set_auto_script, mAutoPath = _msg->Str(2))
    HANDLE_ACTION(set_record_script, mRecordPath = _msg->Str(2))
    HANDLE_ACTION(
        add_message_type, AddMessageType(_msg->Obj<Hmx::Object>(2), _msg->Sym(3))
    )
    if (!mScreenScripts && !mRecord) {
        return DATA_UNHANDLED;
    }
    HANDLE_MESSAGE(UITransitionCompleteMsg)
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_MESSAGE(UIComponentSelectMsg)
    HANDLE_MESSAGE(UIComponentScrollMsg)
    HANDLE_MESSAGE(UIComponentFocusChangeMsg)
    HANDLE_MESSAGE(UIScreenChangeMsg)
    HANDLE(cheat_invoked, OnCheatInvoked)
    HANDLE_METHOD(OnCustomMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

const char *Automator::ToggleAuto() {
    mCurScript = 0;
    if (mScreenScripts) {
        mScreenScripts->Release();
        mScreenScripts = 0;
    } else {
        Loader *ldr = TheLoadMgr.AddLoader(mAutoPath.c_str(), kLoadFront);
        DataLoader *dl = dynamic_cast<DataLoader *>(ldr);
        MILO_ASSERT(dl, 0x90);
        TheLoadMgr.PollUntilLoaded(dl, 0);
        mScreenScripts = dl->Data();
        mCurScreenIndex = 0;
        if (mScreenScripts) {
            StartAuto(mUIManager.CurrentScreen());
        }
    }
    return AutoScript();
}

void Automator::StartAuto(UIScreen *screen) {
    MILO_ASSERT(mScreenScripts, 0xC0);
    mCurScript = nullptr;
    if (screen) {
        mCurMsgIndex = 1;
        for (int i = mCurScreenIndex; i < mScreenScripts->Size(); i++) {
            DataArray *arr = mScreenScripts->Array(i);
            if (arr->Sym(0) == screen->Name()) {
                mCurScript = arr;
                mCurScreenIndex++;
                break;
            }
        }
    }
}

Symbol Automator::CurRecordScreen() {
    DataArray *recordArr = mRecord;
    if (recordArr->Size() > 0) {
        return recordArr->Array(recordArr->Size() - 1)->Sym(0);
    } else
        return gNullStr;
}

void Automator::AddRecord(Symbol s, DataArray *arr) {
    MILO_ASSERT(mRecord, 0x14F);
    int recordSize = mRecord->Size();
    DataArray *addArr;
    if (CurRecordScreen() == s) {
        addArr = mRecord->Array(recordSize - 1);
    } else {
        addArr = new DataArray(1);
        addArr->Node(0) = s;
        mRecord->Insert(recordSize, addArr);
    }
    addArr->Insert(addArr->Size(), arr);
}

void Automator::FinishRecord() {
    if (mRecord) {
        MILO_ASSERT(!mRecordPath.empty(), 0x162);
        DataWriteFile(mRecordPath.c_str(), mRecord, 0);
    }
    if (mRecord) {
        mRecord->Release();
        mRecord = nullptr;
    }
}

DataNode Automator::OnCustomMsg(const Message &msg) {
    Symbol key = msg.Type();
    // ain't no way this is how hmx wrote it
    std::list<Symbol>::iterator it = mCustomMsgs.begin();
    if (it != mCustomMsgs.end()) {
        for (; it != mCustomMsgs.end() && *it != key; ++it)
            ;
        if (it != mCustomMsgs.end())
            HandleMessage(key);
    }
    return DATA_UNHANDLED;
}

DataNode Automator::OnMsg(const UITransitionCompleteMsg &msg) {
    if (mScreenScripts && !mRecord)
        StartAuto(msg.GetNewScreen());
    return DATA_UNHANDLED;
}

void Automator::FillButtonMsg(ButtonDownMsg &msg, int idx) {
    MILO_ASSERT(mCurScript, 0x141);
    DataArray *b = mCurScript->Array(idx);
    static Symbol button_down("button_down");
    MILO_ASSERT(b->Sym(0) == button_down, 0x144);
    int padnum = b->Int(3);
    msg[0] = TheUserMgr->GetLocalUserFromPadNum(padnum);
    msg[1] = b->Int(1);
    msg[2] = b->Int(2);
    msg[3] = padnum;
}

void Automator::AdvanceScript(Symbol msg) {
    if (mCurScript) {
        if (mCurScript->Array(mCurMsgIndex)->Sym(0) == msg) {
            mFramesSinceAdvance = 0;
            mCurMsgIndex++;
            if (mCurMsgIndex >= mCurScript->Size()) {
                mCurScript = 0;
                if (mScreenScripts->Size() == mCurScreenIndex) {
                    static Message auto_script_done("auto_script_done");
                    mUIManager.Handle(auto_script_done, false);
                }
            }
        }
    }
}

char const *Automator::ToggleRecord() {
    if (mRecord != nullptr) {
        FinishRecord();

    } else {
        mSkipNextQuickCheat = true;
        mRecord = new DataArray(0);
    }

    if (mRecord != nullptr)
        return mRecordPath.c_str();
    else
        return "OFF";
}

Symbol Automator::CurScreenName() {
    UIScreen *screen = mUIManager.CurrentScreen();
    if (screen) {
        static Message msg("is_system_cheat");
        DataNode handled = screen->Handle(msg, false);
        if (!handled.Equal(DATA_UNHANDLED, nullptr, true) && handled.Int() != 0) {
            return screen->Name();
        }
    }
    return gNullStr;
}

void Automator::AddMessageType(Hmx::Object *obj, Symbol s2) {
    obj->AddSink(this, s2);
    mCustomMsgs.push_back(s2);
}

void Automator::Poll() {
    static Symbol button_down("button_down");
    static Symbol quick_cheat("quick_cheat");
    static ButtonDownMsg b_msg(nullptr, kPad_NumButtons, kAction_None, -1);
    if (mCurScript) {
        mFramesSinceAdvance++;
        DataArray *scriptArr = mCurScript->Array(mCurMsgIndex);
        Symbol s60 = scriptArr->Sym(0);
        if (s60 == button_down) {
            FillButtonMsg(b_msg, mCurMsgIndex);
            static Symbol button_down("button_down");
            AdvanceScript(button_down);
            mUIManager.Handle(b_msg, false);
        } else if (s60 == quick_cheat) {
            DataArray *a = scriptArr->Array(1);
            AdvanceScript(quick_cheat);
            CallQuickCheat(a, nullptr);
        } else if (mCurMsgIndex > 1 && mFramesSinceAdvance > 0x1E) {
            int prevIdx = mCurMsgIndex - 1;
            if (mCurScript->Array(prevIdx)->Sym(0) == button_down) {
                FillButtonMsg(b_msg, prevIdx);
                mUIManager.Handle(b_msg, false);
            }
        }
    }
}

DataNode Automator::OnMsg(ButtonDownMsg const &msg) {
    Symbol name = CurScreenName();
    if (mRecord && !name.Null()) {
        static Symbol button_down("button_down");
        DataArrayPtr ptr(
            button_down,
            DataGetMacroByInt(msg.GetButton(), "kPad_"),
            DataGetMacroByInt(msg.GetAction(), "kAction_"),
            msg.GetPadNum()
        );
        AddRecord(name, ptr);
    }
    return DATA_UNHANDLED;
}

DataNode Automator::OnCheatInvoked(DataArray const *arr) {
    if (mRecord) {
        if (mSkipNextQuickCheat) {
            mSkipNextQuickCheat = false;
        } else if (arr->Int(2) != 0) {
            Symbol screen = CurScreenName();
            if (mUIManager.CurrentScreen()) {
                if (screen.Null()) {
                    screen = CurRecordScreen();
                }
            }
            if (!screen.Null()) {
                static Symbol quick_cheat("quick_cheat");
                DataArrayPtr ptr(quick_cheat, arr->Array(3));
                AddRecord(screen, ptr);
            }
        }
    }
    return DATA_UNHANDLED;
}

void Automator::HandleMessage(Symbol msgType) {
    if (!mUIManager.InTransition()) {
        if (mRecord) {
            Symbol screenName = CurScreenName();
            if (!screenName.Null()) {
                DataArrayPtr ptr(msgType);
                AddRecord(screenName, ptr);
            }
        } else if (mScreenScripts) {
            AdvanceScript(msgType);
        }
    }
}

#pragma endregion
