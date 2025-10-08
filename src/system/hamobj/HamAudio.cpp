#include "hamobj/HamAudio.h"
#include "math/Utl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "synth/Faders.h"
#include "synth/FxSend.h"
#include "synth/Synth.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/SongInfoCopy.h"

HamAudio::HamAudio()
    : mFileLoader(0), unk34(0), mSongInfo(0), mSongStream(0), unk4c(0),
      mMasterFader(Hmx::Object::New<Fader>()), mMuteMaster(0), unk59(0), unk68(0),
      unk78(0) {
    unk44[0] = 0;
    unk44[1] = 0;
    mCrossFaders[0] = Hmx::Object::New<Fader>();
    mCrossFaders[1] = Hmx::Object::New<Fader>();
}

HamAudio::~HamAudio() {
    Clear();
    RELEASE(mMasterFader);
    RELEASE(mCrossFaders[0]);
    RELEASE(mCrossFaders[1]);
}

BEGIN_HANDLERS(HamAudio)
    HANDLE_ACTION(toggle_mute_master, ToggleMuteMaster())
    HANDLE_ACTION(set_mute_master, SetMuteMaster(_msg->Int(2)))
    HANDLE_ACTION(print_faders, PrintFaders())
    HANDLE_EXPR(num_channels, (int)unk84.size())
    HANDLE_ACTION(set_channel_volume, SetChannelVolume(_msg->Int(2), _msg->Float(3)))
    HANDLE_ACTION_IF(
        set_track_volume,
        unk90[_msg->Sym(2)],
        unk90[_msg->Sym(2)]->SetVolume(_msg->Float(3))
    )
    HANDLE_ACTION(set_loop, SetLoop(_msg->Float(2), _msg->Float(3)))
    HANDLE_ACTION(clear_loop, ClearLoop())
    HANDLE(get_loop_beats, OnGetCurrentLoopBeats)
    HANDLE_ACTION(jump, Jump(_msg->Float(2)))
    HANDLE(set_crossfade_jump, OnSetCrossfadeJump)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamAudio)
    if (GetSongStream()) {
        SYNC_PROP_SET(
            speed, GetSongStream()->GetSpeed(), GetSongStream()->SetSpeed(_val.Float())
        )
    } else {
        SYNC_PROP_SET(speed, 1.0f, )
    }
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

bool HamAudio::IsReady() {
    if (!mSongStream && !unk34) {
        if (mFileLoader && mFileLoader->IsLoaded()) {
            FinishLoad();
        } else
            return false;
    }
    unk4c = mSongStream && mSongStream->IsReady();
    return unk4c;
}

bool HamAudio::Paused() const { return !(mSongStream && mSongStream->IsPlaying()); }

void HamAudio::SetPaused(bool pause) {
    if (mSongStream) {
        if (pause) {
            mSongStream->Stop();
        } else if (!mSongStream->IsPlaying()) {
            mSongStream->Play();
        }
    }
}

void HamAudio::Poll() {
    if (gMiloTool && mFileLoader && !mFileLoader->IsLoaded()) {
        mFileLoader->PollLoading();
    }
    PollCrossfade();
}

float HamAudio::GetTime() const {
    if (mSongStream) {
        return mSongStream->GetTime();
    }
    return 0;
}

void HamAudio::SetMasterVolume(float vol) {
    mMasterVolume = vol;
    UpdateMasterFader();
}

void HamAudio::SetChannelVolume(int channel, float volume) {
    unk84[channel]->SetVolume(volume);
}

void HamAudio::SetMuteMaster(bool mute) {
    mMuteMaster = mute;
    UpdateMasterFader();
}

void HamAudio::ToggleMuteMaster() {
    mMuteMaster = !mMuteMaster;
    UpdateMasterFader();
}

void HamAudio::UpdateMasterFader() {
    float masterVolume;
    if (mMuteMaster != 0) {
        masterVolume = kDbSilence;
    } else {
        masterVolume = mMasterVolume;
    }
    mMasterFader->SetVolume(masterVolume);
}

bool HamAudio::Fail() { return mSongStream && mSongStream->Fail(); }
bool HamAudio::IsFinished() const { return mSongStream && mSongStream->IsFinished(); }

void HamAudio::Jump(float f1) {
    if (mSongStream) {
        mSongStream->Stop();
        mCrossFaders[0]->SetVolume(0);
        mCrossFaders[1]->SetVolume(kDbSilence);
        unk78 = 0;
        if (unk44[1]) {
            unk44[1]->Stop();
        }
        unk4c = false;
        mSongStream->Resync(f1);
    }
}

void HamAudio::ClearLoop() {
    if (GetSongStream()) {
        GetSongStream()->ClearJump();
    }
    unk68 = 0;
}

void HamAudio::DeleteFaders() {
    DeleteAll(unk84);
    for (std::map<Symbol, Fader *>::iterator it = unk90.begin(); it != unk90.end();
         ++it) {
        RELEASE(it->second);
    }
    unk90.clear();
}

void HamAudio::Clear() {
    if (mSongStream) {
        for (int i = 0; i < 2; i++) {
            mSongStream->SetFX(i, false);
        }
    }
    RELEASE(unk44[0]);
    RELEASE(unk44[1]);
    mSongStream = nullptr;
    RELEASE(mFileLoader);
    if (unk34) {
        MemFree(unk34);
        unk34 = nullptr;
        unk38 = 0;
    }
    mSongInfo = nullptr;
    DeleteFaders();
    unk68 = 0;
    unk78 = 0;
}

void HamAudio::Load(SongInfo *info, bool b2) {
    Clear();
    mSongInfo = info;
    String str(info->GetBaseFileName());
    if (b2) {
        Stream *stream = TheSynth->NewStream(str.c_str(), 0, 0, false);
        mSongStream = stream;
        unk44[0] = stream;
        FinishLoad();
    } else {
        String moggStr(MakeString("%s.mogg", str.c_str()));
        mFileLoader =
            new FileLoader(moggStr.c_str(), "main", kLoadFront, 0, false, true, 0, 0);
    }
}

void HamAudio::Play() {
    MILO_ASSERT(mSongStream, 0x11B);
    mSongStream->Play();
    if (!unk59) {
        if (TheSynth->CheckCommonBank(false)) {
            FxSend *send = TheSynth->Find<FxSend>("song.send", false);
            if (send) {
                for (int i = 0; i < 2; i++) {
                    if (unk44[i]) {
                        for (int j = 0; j < unk44[i]->GetNumChannels(); j++) {
                            unk44[i]->SetFXSend(j, send);
                        }
                    }
                }
                unk59 = true;
            }
        }
    }
}

void HamAudio::PrintFaders() {
    MILO_LOG("MasterFader %.2f\n", mMasterFader->DuckedValue());
    MILO_LOG("CrossFaders[0] %.2f\n", mCrossFaders[0]->DuckedValue());
    MILO_LOG("CrossFaders[1] %.2f\n", mCrossFaders[1]->DuckedValue());
}
