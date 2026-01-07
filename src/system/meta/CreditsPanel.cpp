#include "CreditsPanel.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "os/System.h"
#include "rndobj/Mat.h"
#include "synth/Stream.h"
#include "synth/Synth.h"
#include "ui/PanelDir.h"
#include "ui/UI.h"
#include "ui/UILabel.h"
#include "ui/UIList.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "ui/UIListProvider.h"
#include "ui/UIPanel.h"
#include "utl/ChunkStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

#pragma region Hmx::Object

CreditsPanel::CreditsPanel()
    : mLoader(0), mNames(0), mStream(0), mAutoScroll(1), mSavedSpeed(-1.0f), mPaused(0) {}

CreditsPanel::~CreditsPanel() {}

BEGIN_HANDLERS(CreditsPanel)
    HANDLE_ACTION(pause_panel, PausePanel(_msg->Int(2)))
    HANDLE_EXPR(is_cheat_on, mCheatOn)
    HANDLE_ACTION(debug_toggle_autoscroll, DebugToggleAutoScroll())
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS

#pragma endregion
#pragma region UIListProvider

void CreditsPanel::Text(int i, int j, UIListLabel *listlabel, UILabel *label) const {
    DataArray *arr = mNames->Array(j);
    MILO_ASSERT(label, 0xF2);
    label->SetCreditsText(arr, listlabel);
}

RndMat *CreditsPanel::Mat(int i, int j, UIListMesh *mesh) const {
    static Symbol image("image");
    static Symbol blank("blank");
    DataArray *array = mNames->Array(j);
    Symbol imgSym = blank;
    if (array->Size() != 0) {
        imgSym = array->Sym(0);
    }
    if (imgSym == image) {
        return mDir->Find<RndMat>(array->Str(1), true);
    } else {
        return nullptr;
    }
}

int CreditsPanel::NumData() const { return mNames->Size(); }

#pragma endregion
#pragma region UIPanel

void CreditsPanel::Load() {
    UIPanel::Load();
    mLoader = new DataLoader(SystemConfig("credits_file")->Str(1), kLoadFront, true);
}

void CreditsPanel::Enter() {
    UIPanel::Enter();
    mCheatOn = false;
    mPaused = false;
    mList->SetSelected(0, -1);
    mAutoScroll = 1;
    mList->AutoScroll();
}

void CreditsPanel::Exit() {
    if (mStream && !mPaused) {
        mStream->Faders()->FindLocal("fade", true)->DoFade(-96.0f, 2000.0f);
    }
    UIPanel::Exit();
}

bool CreditsPanel::Exiting() const {
    return mStream && mStream->Faders()->FindLocal("fade", true)->IsFading()
        || UIPanel::Exiting();
}

void CreditsPanel::Poll() {
    UIPanel::Poll();
    if (!mStream) {
        DataArray *cfg = SystemConfig("sound", "credits");
        String streamStr;
        cfg->FindData("stream", streamStr, true);
        float volume;
        cfg->FindData("volume", volume, true);
        mStream = TheSynth->NewStream(streamStr.c_str(), 0, 0, 0);
        mStream->SetPan(0, -1);
        mStream->SetPan(1, 1);
        mStream->SetVolume(volume);
        mStream->Faders()->AddLocal("fade");
        mStream->SetJump(Stream::kStreamEndMs, 0, 0);
    } else if (!mStream->IsPlaying() && mStream->IsReady() && !mPaused) {
        mStream->Play();
    }
    if (mAutoScroll && !TheUI->InTransition()) {
        if (!mList->IsScrolling()) {
            static Message cMsg("credits_done");
            HandleType(cMsg);
            SetAutoScroll(false);
        }
    }
}

bool CreditsPanel::IsLoaded() const {
    return UIPanel::IsLoaded() && mLoader != nullptr && mLoader->IsLoaded();
}

void CreditsPanel::Unload() {
    UIPanel::Unload();
    if (mNames) {
        mNames->Release();
        mNames = 0;
    }
    RELEASE(mStream);
}

void CreditsPanel::FinishLoad() {
    UIPanel::FinishLoad();
    mNames = mLoader->Data();
    MILO_ASSERT(mNames, 0x35);
    mNames->AddRef();
    mList = mDir->Find<UIList>("credits.lst", true);
    mList->SetProvider(this);
    delete mLoader;
    mLoader = 0;
}

#pragma endregion
#pragma region CreditsPanel

void CreditsPanel::PausePanel(bool b) {
    if (mPaused != b) {
        mPaused = b;
        if (mPaused) {
            SetAutoScroll(false);
            if (mStream && !mStream->IsPaused()) {
                mStream->Stop();
            }
        } else {
            SetAutoScroll(true);
            if (mStream && mStream->IsPaused()) {
                mStream->Play();
            }
        }
    }
}

void CreditsPanel::DebugToggleAutoScroll() {
    if (!mAutoScroll) {
        mList->SetSpeed(mSavedSpeed);
        SetAutoScroll(true);
        mCheatOn = false;
    } else {
        mSavedSpeed = mList->Speed();
        mList->SetSpeed(0.0f);
        SetAutoScroll(false);
        mCheatOn = true;
    }
}

DataNode CreditsPanel::OnMsg(const ButtonDownMsg &msg) {
    if (mAutoScroll)
        return DATA_UNHANDLED;
    if (msg.GetButton() == kPad_DDown || msg.GetButton() == kPad_LStickDown) {
        mList->Scroll(1);
    } else if (msg.GetButton() == kPad_DUp || msg.GetButton() == kPad_LStickUp) {
        mList->Scroll(-1);
    }
    return 1;
}

void CreditsPanel::SetAutoScroll(bool b) {
    if (b != mAutoScroll) {
        mAutoScroll = b;
        if (mAutoScroll)
            mList->AutoScroll();
        else
            mList->StopAutoScroll();
    }
}
