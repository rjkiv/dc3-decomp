#include "ui/UI.h"
#include "UIComponent.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/MessageTimer.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/JoypadClient.h"
#include "os/UserMgr.h"
#include "ui/CheatProvider.h"
#include "ui/UILabel.h"
#include "ui/UIScreen.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

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

#pragma region UIManager

UIManager::UIManager()
    : mWentBack(0), mMaxPushDepth(100), mJoyClient(0), mCurrentScreen(0), unk50(0),
      mOverloadHorizontalNav(0), mCancelTransitionNotify(0), mDefaultAllowEditText(1),
      mDisableScreenBlacklight(0), mOverlay(0), mAutomator(0), unkd0(0) {}

UIManager::~UIManager() {}

void UIManager::SetScreenBlacklghtDisabled(bool disable) {
    mDisableScreenBlacklight = disable;
}

bool UIManager::InComponentSelect() {
    if (mCurrentScreen)
        return mCurrentScreen->InComponentSelect();
    else
        return false;
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

void UIManager::Draw() {
    for (std::vector<UIScreen *>::iterator it = mPushedScreens.begin();
         it != mPushedScreens.end();
         ++it) {
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

bool UIManager::OverloadHorizontalNav(JoypadAction, JoypadButton, bool) const {
    return false;
}

void UIManager::Terminate() {
    CheatProvider::Terminate();
    SetName(0, 0);
    KeyboardUnsubscribe(this);
    RELEASE(mCam);
    RELEASE(mEnv);
    RELEASE(mJoyClient);
    TheDebug.RemoveExitCallback(TerminateCallback);
    RELEASE(mAutomator);
}

bool UIManager::IsGameScreenActive() { return false; }

bool UIManager::BlockHandlerDuringTransition(Symbol s, DataArray *da) { return false; }

void UIManager::GotoScreenImpl(UIScreen *scr, bool b1, bool b2) {
    if (b1 || mTransitionState != kTransitionNone
        || mCurrentScreen != scr
            && (mTransitionState != kTransitionTo && mTransitionState != kTransitionPop)
        || mTransitionScreen != scr) {
        CancelTransition();

        if (scr) {
            for (std::vector<UIScreen *>::iterator it = mPushedScreens.begin();
                 it != mPushedScreens.end();
                 ++it) {
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

DataNode UIManager::OnIsResource(DataArray *arr) { return 0; }

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
    return DataNode(kDataUnhandled, 0);
}

void UIManager::ReloadStrings() {}

void UIManager::FakeKeyboardAction(JoypadButton, JoypadAction) {}

void UIManager::Poll() {}

void UIManager::PushScreen(UIScreen *screen) { MILO_ASSERT(screen, 0x38c); }

DataNode UIManager::OnForeachCurrentScreen(DataArray const *) { return NULL_OBJ; }

void UIManager::Init() {}

BEGIN_HANDLERS(UIManager)
END_HANDLERS

#pragma endregion UIManager
#pragma region AutoMator

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
    return DataNode(kDataUnhandled, 0);
}

DataNode Automator::OnMsg(const UITransitionCompleteMsg &msg) {
    if (mScreenScripts && !mRecord)
        StartAuto(msg.GetNewScreen());
    return DataNode(kDataUnhandled, 0);
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

Symbol Automator::CurScreenName() { return gNullStr; }

void Automator::Poll() {}

DataNode Automator::OnMsg(ButtonDownMsg const &msg) {
    return DataNode(kDataUnhandled, 0);
}

DataNode Automator::OnCheatInvoked(DataArray const *arr) {
    return DataNode(kDataUnhandled, 0);
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

BEGIN_HANDLERS(Automator)

END_HANDLERS

#pragma endregion Automator
