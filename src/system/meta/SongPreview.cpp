#include "meta/SongPreview.h"
#include "SongMetadata.h"
#include "SongMgr.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/System.h"
#include "synth/Faders.h"
#include "utl/SongInfoCopy.h"
#include "utl/Symbol.h"

SongPreview::SongPreview(const SongMgr &mgr) 
    : mSongMgr(mgr), unk34(0), unk38(this), unk4c(0), unk50(0), unk54(0), unk58(0), 
    unk5c(0), unk60(0.0f), unk68(0.0f), unk70(0), unk7c(0.0f), unk80(0.0f), unk84(0.0f), 
    unk88(0.0f), unk8c(0), unk8d(0), unk8e(0) {}

void SongPreview::ContentMounted(char const * c, char const * cc2){
    if(c==nullptr){
        MILO_FAIL(kAssertStr,"SongPreview.cpp",0xbf,"contentName");
    }
    Symbol s = Symbol(c);
    if(s==unk78){
        unk78 = 0;
    }
}

void SongPreview::ContentFailed(char const * c){
    if(c==nullptr){
        MILO_FAIL(kAssertStr,"SongPreview.cpp",0xcb,"contentName");
    }
    Symbol sym = c;
    if(sym==unk78){
        unk74 = 0;
        unk70 = 0;
        unk78 = 0;
    }
}

SongPreview::~SongPreview(){ Terminate(); }

bool SongPreview::IsWaitingToDelete() const{
    return unk70==3;
}

bool SongPreview::IsFadingOut() const{
    return unk70==5;
}

void SongPreview::SetMusicVol(float f){
    if(unk4c==0){
        return;
    }
    if(f<unk54->GetMLevelTarget()){
        unk54->DoFade(f,250.0f);
    }
    else {
        unk54->DoFade(f,1000.0f);
    }
}

void SongPreview::Init(){
    if(unk4c){
        unk4c=true;
        unk74=0;
        unk78 = 0;
        if(unk34){
            //do something
        }
        unk34=0;
        RELEASE(unk38);
        unk70=0;
        unk6c=true;
        DataArray *cfg = SystemConfig("sound","song_select");
        cfg->FindData("loop_forever",unk6d,true);
        cfg->FindData("fade_time",unk64,true);
        cfg->FindData("attenuation",unk60,true);
        cfg->FindData("preview_db",unk68,true);
        unk64*=1000.0f;
        unk50=Hmx::Object::New<Fader>();
        unk54=Hmx::Object::New<Fader>();
        unk58=Hmx::Object::New<Fader>();
        unk58->SetVolume(-96.0f);
    }
}

void SongPreview::Terminate(){
    if(unk4c){
        unk4c=0;
        DetachFader(unk54);
        DetachFader(unk58);
        unk74 = 0;
        unk78 = 0;
        RELEASE(unk34);
        RELEASE(unk50);
        RELEASE(unk54);
        RELEASE(unk58);

        if(unk8c){
            TheContentMgr->UnregisterCallback(this, true);
            unk8c=0;
        }
    }
}

void SongPreview::Start(Symbol s, TexMovie *t){
    if(unk4c || s){
        if(unk50==0||unk54==0||unk58==0){
            MILO_FAIL(kAssertStr,"SongPreview.cpp",0x6c,"mFader && mMusicFader && mCrowdSingFader");
        }
        //unk38->SetObjConcrete(t);
        if(s==unk74){
            unk8d=true;
        }
        else{
            if(s){
                if(!mSongMgr.HasSong(s,false)){
                    return;
                }   
                const SongMetadata *data = mSongMgr.Data(mSongMgr.GetSongIDFromShortName(s, true));
                if(data && !data->IsVersionOK()){
                    s = gNullStr;
                }
                if(unk8c){
                    TheContentMgr->RegisterCallback(this, false);
                    unk8c=true;
                }
            }
            unk6c=true;
            unk54->SetVolume(unk68);
            unk58->SetVolume(-96.0f);
            int x;
            if(unk70 < 2){
                if(unk34){
                    //do something
                }
                x=0;
                unk34=0;
            }
            else if(unk70==2){
                x=3;
            }
            else{
                if(unk70 !=4)
                    return;
                else{
                    unk50->DoFade(-48.0f);
                }
            }
            unk70=x;
        }
    }
}

void SongPreview::PreparePreview(){
    float previewstart = 0.0f;
    float previewend = 15000.0f;
    if(unk84!=0.0f || unk88!=0.0f){
        previewend = unk84;
        previewstart = unk88;
    }
    else{
        int songid = mSongMgr.GetSongIDFromShortName(unk74, true);
        mSongMgr.Data(songid)->PreviewTimes(previewstart, previewend);
    }
    unk7c = previewstart;
    unk80 = previewend;
    PrepareSong(unk74);
    
}

void SongPreview::Poll(){
    
}

DataNode SongPreview::OnStart(DataArray * arr){
    return NULL_OBJ;
}

void SongPreview::DetachFader(Fader* f){
    if(unk34 && f){
        for(int i=0;i<unk5c;i++){
            unk34->ChannelFaders(i)->Remove(f);
        }
    }
}

void SongPreview::PrepareFaders(SongInfo const * info){
    for(int i=0;i<unk5c;i++){
        FaderGroup *f = unk34->ChannelFaders(i);
        f->Add(unk54);
    }
}

void SongPreview::PrepareSong(Symbol s){

}

BEGIN_HANDLERS(SongPreview)
//     HANDLE(start, OnStart)
//     HANDLE_ACTION(start_video, action)
//     HANDLE_ACTION(set_music_vol, SetMusicVol(_msg->Float(2)))
//     HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
