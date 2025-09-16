#include "CreditsPanel.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "os/Debug.h"
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
#include "ui/UIPanel.h"
#include "utl/ChunkStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

CreditsPanel::CreditsPanel()
    : mLoader(0), mNames(0), mStream(0), mAutoScroll(1), mSavedSpeed(-1.0f), mPaused(0) {}

CreditsPanel::~CreditsPanel(){}

void CreditsPanel::Text(int i, int j, UIListLabel *listlabel, UILabel *label) const{
    DataArray *arr = mNames->Array(j);
    MILO_ASSERT(label, 0xF2);
    label->SetCreditsText(arr, listlabel);
}

int CreditsPanel::NumData() const{
    return mNames->Size();
}

RndMat *CreditsPanel::Mat(int i, int j, UIListMesh *mesh) const{
    return nullptr;
}

DataNode CreditsPanel::OnMsg(ButtonDownMsg const &msg){
    return NULL_OBJ;
}

bool CreditsPanel::IsLoaded() const{
    return UIPanel::IsLoaded() && mLoader->IsLoaded();
}

void CreditsPanel::Exit(){
    if(mStream && !mPaused){
        mStream->Faders()->FindLocal("fade", true)->DoFade(-96.0f, 2000.0f);
    }
    UIPanel::Exit();
}

void CreditsPanel::Enter(){
    UIPanel::Enter();

        mCheatOn = false;

        mPaused=false;
        //mList->SetSelectedAux(1);
        mAutoScroll=true;
        //mList->AutoScroll();
}

void CreditsPanel::PausePanel(bool b){
    
}

void CreditsPanel::DebugToggleAutoScroll(){
    /*if(!mAutoScroll){
        mList->SetSpeed(mSavedSpeed);
        SetAutoScroll(true);
        mCheatOn = false;
    }
    else{
        mSavedSpeed = mList->Speed();
        mList->SetSpeed(0.0f);
        SetAutoScroll(false);
        mCheatOn=true;
    }*/
}


void CreditsPanel::Load(){
    UIPanel::Load();
    //mLoader = new DataLoader(FilePath(SystemConfig("credits_file")->Str(1)), kLoadFront, true); 
}

bool CreditsPanel::Exiting() const{
    return mStream && mStream->Faders()->FindLocal("fade", true) || UIPanel::Exiting();
}

void CreditsPanel::FinishLoad(){
    UIPanel::FinishLoad();
    mNames = mLoader->Data();
    MILO_ASSERT(mNames, 0x35);
    mNames->AddRef();
    //mList = mDir->Find<UIList>("credits.lst", true);
    //mList->SetProvider(this);
    delete mLoader;
    mLoader = 0;
}

void CreditsPanel::Unload(){
    UIPanel::Unload();
    if(mNames){
        mNames->Release();
        mNames = 0;
    }
    RELEASE(mStream);
}

void CreditsPanel::Poll(){
    UIPanel::Poll();
    if(!mStream){
        mStream=TheSynth->NewStream("sfx/streams/credits", 0, 0, 0);
        MILO_ASSERT_FMT(mStream, "sfx/streams/credits.foo missing");
        //mStream->SetJump(Stream::kStreamEndMs, 0, 0);
        mStream->SetPan(0, -1.0f);
        mStream->SetPan(1, 1.0f);
        mStream->SetVolume(-4.0f);
        mStream->Faders()->AddLocal("fade");
    }
    else{
        if (!mStream->IsPlaying() && mStream->IsReady() && !mPaused) {
            mStream->Play();
        }
    }
    /*if (mAutoScroll && !TheUI->InTransition()) {
        if (!mList->IsScrolling()) {
            HandleType(credits_done_msg);
            SetAutoScroll(false);
        }
    }*/
}

BEGIN_HANDLERS(CreditsPanel)
    HANDLE_ACTION(pause_panel, PausePanel(_msg->Int(2)))
    HANDLE_EXPR(is_cheat_on, mCheatOn)
    HANDLE_EXPR(is_cheat_on, false)
    HANDLE_ACTION(debug_toggle_autoscroll, DebugToggleAutoScroll())
    HANDLE_MESSAGE(ButtonDownMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
