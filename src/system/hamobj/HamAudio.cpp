#include "hamobj/HamAudio.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "synth/Faders.h"
#include "synth/FxSend.h"
#include "synth/Stream.h"
#include "synth/Synth.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/SongInfoAudioType.h"
#include "utl/SongInfoCopy.h"
#include "utl/TimeConversion.h"

HamAudio::HamAudio()
    : mFileLoader(0), mAudioData(0), mCurrSongInfo(0), mSongStream(0), mStreamEnabled(0),
      mMasterFader(Hmx::Object::New<Fader>()), mMuteMaster(0), mInitSend(0) {
    mSongStreams[0] = 0;
    mSongStreams[1] = 0;
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
    HANDLE_EXPR(num_channels, (int)mChannels.size())
    HANDLE_ACTION(set_channel_volume, SetChannelVolume(_msg->Int(2), _msg->Float(3)))
    HANDLE_ACTION_IF(
        set_track_volume,
        mTracks[_msg->Sym(2)],
        mTracks[_msg->Sym(2)]->SetVolume(_msg->Float(3))
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
    if (!mSongStream && !mAudioData) {
        if (mFileLoader && mFileLoader->IsLoaded()) {
            FinishLoad();
        } else
            return false;
    }
    mStreamEnabled = mSongStream && mSongStream->IsReady();
    return mStreamEnabled;
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
    mChannels[channel]->SetVolume(volume);
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
        mActiveCrossfade.mXstate = CFS_UNSET;
        if (mSongStreams[1]) {
            mSongStreams[1]->Stop();
        }
        mStreamEnabled = false;
        mSongStream->Resync(f1);
    }
}

void HamAudio::ClearLoop() {
    if (GetSongStream()) {
        GetSongStream()->ClearJump();
    }
    mQueuedCrossfade.mXstate = CFS_UNSET;
}

void HamAudio::DeleteFaders() {
    DeleteAll(mChannels);
    FOREACH (it, mTracks) {
        RELEASE(it->second);
    }
    mTracks.clear();
}

void HamAudio::Clear() {
    if (mSongStream) {
        for (int i = 0; i < 2; i++) {
            mSongStream->SetFX(i, false);
        }
    }
    RELEASE(mSongStreams[0]);
    RELEASE(mSongStreams[1]);
    mSongStream = nullptr;
    RELEASE(mFileLoader);
    if (mAudioData) {
        MemFree(mAudioData);
        mAudioData = nullptr;
        mAudioDataSize = 0;
    }
    mCurrSongInfo = nullptr;
    DeleteFaders();
    mQueuedCrossfade.mXstate = CFS_UNSET;
    mActiveCrossfade.mXstate = CFS_UNSET;
}

void HamAudio::Load(SongInfo *song_info, bool stream) {
    Clear();
    mCurrSongInfo = song_info;
    String str(song_info->GetBaseFileName());
    if (stream) {
        Stream *stream = TheSynth->NewStream(str.c_str(), 0, 0, false);
        mSongStream = stream;
        mSongStreams[0] = stream;
        FinishLoad();
    } else {
        String moggStr(MakeString("%s.mogg", str.c_str()));
        mFileLoader = new FileLoader(
            moggStr.c_str(), moggStr.c_str(), kLoadFront, 0, false, true, 0, "main"
        );
    }
}

void HamAudio::Play() {
    MILO_ASSERT(mSongStream, 0x11B);
    mSongStream->Play();
    if (!mInitSend) {
        if (TheSynth->CheckCommonBank(false)) {
            FxSend *send = TheSynth->Find<FxSend>("song.send", false);
            if (send) {
                for (int i = 0; i < 2; i++) {
                    if (mSongStreams[i]) {
                        for (int j = 0; j < mSongStreams[i]->GetNumChannels(); j++) {
                            mSongStreams[i]->SetFXSend(j, send);
                        }
                    }
                }
                mInitSend = true;
            }
        }
    }
}

void HamAudio::PrintFaders() {
    MILO_LOG("MasterFader %.2f\n", mMasterFader->DuckedValue());
    MILO_LOG("CrossFaders[0] %.2f\n", mCrossFaders[0]->DuckedValue());
    MILO_LOG("CrossFaders[1] %.2f\n", mCrossFaders[1]->DuckedValue());
}

// intentionally unimplemented
void HamAudio::SetBackgroundVolume(float) {}
void HamAudio::SetForegroundVolume(float) {}
void HamAudio::SetStereo(bool) {}

bool HamAudio::GetCurrLoopMarkers(float &f1, float &f2) const {
    Marker m1, m2;
    Stream *s = mSongStream;
    if (!s || !s->CurrentJumpPoints(m2, m1)) {
        return false;
    }
    f1 = m1.posMS;
    f2 = m2.posMS;
    return true;
}

bool HamAudio::GetCurrLoopBeats(int &startBeats, int &endBeats) const {
    float f1, f2;
    if (!GetCurrLoopMarkers(f1, f2)) {
        return false;
    } else {
        startBeats = SecondsToBeat(f1 / 1000.0f) + 0.5f;
        endBeats = SecondsToBeat(f2 / 1000.0f) + 0.5f;
        return true;
    }
}

void HamAudio::SetLoop(float startBeat, float endBeat) {
    SetLoop(BeatToMs(startBeat), BeatToMs(endBeat), GetSongStream());
}

void HamAudio::SetCrossfadeJump(float fromMs, float toMs, float fadeMs) {
    MILO_ASSERT_FMT(
        mSongStreams[0] && mSongStreams[1], "Crossfade requires 2 song streams"
    );
    if (mQueuedCrossfade.mXstate != CFS_UNSET) {
        MILO_NOTIFY("Stomping on current queued crossfade");
    }
    Crossfade &fade = mQueuedCrossfade;
    fade.fadeMs = fadeMs;
    fade.mXtoMs = toMs;
    fade.mXfromMs = fromMs;
    fade.mXstate = CFS_READY;
    bool skip = false;
    if (fade.mXfromMs - fade.fadeMs / 2 <= 0) {
        MILO_NOTIFY(
            "Crossfade begins before start of song. Setting up hard jump instead of crossfade."
        );
        skip = true;
    }
    if (mActiveCrossfade.mXstate > CFS_READY
        && fade.mXfromMs - fade.fadeMs / 2
            <= mActiveCrossfade.fadeMs / 2 + mActiveCrossfade.mXtoMs) {
        MILO_NOTIFY(
            "Crossfade begins before existing crossfade ends. Setting up hard jump instead of crossfade."
        );
        skip = true;
    }
    if (skip) {
        mQueuedCrossfade.mXstate = CFS_UNSET;
    }
    SetLoop(toMs, fromMs, mSongStreams[0]);
}

void HamAudio::SetLoop(float startMs, float endMs, Stream *stream) {
    Marker m1, m2;
    if (stream->CurrentJumpPoints(m2, m1) && m1.posMS == startMs && m2.posMS == endMs) {
        return;
    } else {
        stream->ClearJump();
        stream->ClearMarkerList();
        String start = "start";
        String end = "end";
        m1.name = start;
        m1.posMS = startMs;
        m2.name = end;
        m2.posMS = endMs;
        stream->AddMarker(m1);
        stream->AddMarker(m2);
        stream->SetJump(end, start);
    }
}

void HamAudio::FinishLoad() {
    if (mFileLoader) {
        mAudioData = mFileLoader->GetBuffer(&mAudioDataSize);
        RELEASE(mFileLoader);
        mSongStreams[0] =
            TheSynth->NewBufStream(mAudioData, mAudioDataSize, "mogg", 0.25f, true);
        mSongStreams[1] =
            TheSynth->NewBufStream(mAudioData, mAudioDataSize, "mogg", 0.25f, true);
        mSongStream = mSongStreams[0];
    }

    for (int i = 0; i < 2; i++) {
        if (mSongStreams[i]) {
            mSongStreams[i]->Faders().Add(mMasterFader);
            mSongStreams[i]->Faders().Add(mCrossFaders[i]);
            mCrossFaders[i]->SetVolume(0);
            const auto &vols = mCurrSongInfo->GetVols();
            const auto &pans = mCurrSongInfo->GetPans();
            int numChannels = vols.size();
            MILO_ASSERT(pans.size() == numChannels, 0x9D);
            for (int c = 0; c < numChannels; c++) {
                Fader *curFader;
                if (c >= mChannels.size()) {
                    curFader = Hmx::Object::New<Fader>();
                    curFader->SetVolume(vols[c]);
                    mChannels.push_back(curFader);
                } else {
                    curFader = mChannels[c];
                }
                mSongStreams[i]->ChannelFaders(c).Add(curFader);
                mSongStreams[i]->SetPan(c, pans[c]);
            }
            const auto &trackChannels = mCurrSongInfo->GetTracks();
            for (int tc = 0; tc < trackChannels.size(); tc++) {
                SongInfoAudioType audioType = trackChannels[tc].mAudioType;
                Symbol audioSym = SongInfoAudioTypeToSym(audioType);
                Fader *trackFader;
                if (mTracks.find(audioSym) == mTracks.end()) {
                    trackFader = Hmx::Object::New<Fader>();
                    mTracks[audioSym] = trackFader;
                } else {
                    trackFader = mTracks[audioSym];
                }
                auto &channels = trackChannels[tc].mChannels;
                for (int c = 0; c < channels.size(); c++) {
                    mSongStreams[i]->ChannelFaders(channels[c]).Add(trackFader);
                }
                if (TheSynth->CheckCommonBank(false)) {
                    Fader *vocalFader = TheSynth->Find<Fader>("vocals_level.fade", false);
                    if (vocalFader && audioType == kAudioTypeVocals) {
                        for (int c = 0; c < channels.size(); c++) {
                            mSongStreams[i]->ChannelFaders(channels[c]).Add(vocalFader);
                        }
                    }
                    Fader *multiFader = TheSynth->Find<Fader>("multi_level.fade", false);
                    if (multiFader && audioType == kAudioTypeMulti) {
                        for (int c = 0; c < channels.size(); c++) {
                            mSongStreams[i]->ChannelFaders(channels[c]).Add(multiFader);
                        }
                    }
                    FxSend *songSend = TheSynth->Find<FxSend>("song.send", false);
                    if (songSend) {
                        for (int c = 0; c < numChannels; c++) {
                            mSongStreams[i]->SetFXSend(c, songSend);
                        }
                        mInitSend = true;
                    }
                }
            }
        }
    }
    if (mSongStreams[1]) {
        if (mSongStreams[1]->IsReady()) {
            mSongStreams[1]->Resync(10000);
        } else {
            MILO_NOTIFY(
                "HamAudio::FinishLoad() - almost tried to resync stream before it was ready"
            );
        }
    }
}

DataNode HamAudio::OnSetCrossfadeJump(DataArray *a) {
    float a2 = a->Float(2);
    float f6 = BeatToMs(a2);
    float f7 = BeatToMs(a->Float(3));
    float f8;
    if (a->Size() > 4) {
        f8 = a->Float(4);
    } else {
        f8 = SystemConfig("synth", "crossfade_beats")->Float(1);
    }
    f8 = BeatToMs(f8 + a2) - f6;
    SetCrossfadeJump(f6, f7, f8);
    return 0;
}

DataNode HamAudio::OnGetCurrentLoopBeats(DataArray *a) {
    int i40, i3c;
    if (!GetCurrLoopBeats(i40, i3c)) {
        return 0;
    } else {
        *a->Var(2) = i40;
        *a->Var(3) = i3c;
        return 1;
    }
}
