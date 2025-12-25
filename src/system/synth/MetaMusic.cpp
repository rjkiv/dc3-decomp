#include "synth/MetaMusic.h"
#include "beatmatch/HxAudio.h"
#include "beatmatch/HxMaster.h"
#include "math/Utl.h"
#include "meta/DataArraySongInfo.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "obj/Utl.h"
#include "os/System.h"
#include "synth/FxSend.h"
#include "synth/Stream.h"
#include "utl/Loader.h"
#include "os/Debug.h"
#include "synth/Synth.h"
#include "synth/FxSendEQ.h"
#include "os/System.h"
#include "utl/Std.h"
#include <cstdio>

MetaMusic *TheMetaMusic;

MetaMusic::MetaMusic(HxMaster *hx, const char *filename)
    : unk2c(0), unk2d(0), unk30(0), mFadeTime(1), mMuteFadeTime(1), mVolume(0),
      mExtraFaders(this), unk60(filename), mStarted(0), unk8c(0), unk98(1), unka8(0),
      unka9(0), mSongInfo(0), unkb0(hx) {
    mFader = Hmx::Object::New<Fader>();
    mFaderMute = Hmx::Object::New<Fader>();
}

MetaMusic::~MetaMusic() {
    UnloadStreamFx();
    delete mFader;
    delete mFaderMute;
    delete mSongInfo;
}

BEGIN_HANDLERS(MetaMusic)
    HANDLE_ACTION(stop, Stop())
    HANDLE_ACTION(start, Start())
    HANDLE_ACTION(poll, Poll())
    HANDLE_EXPR(is_active, IsActive())
    HANDLE_EXPR(is_started, IsStarted())
    HANDLE_ACTION(set_quiet_vol, SetQuietVolume(_msg->Float(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void MetaMusic::SetQuietVolume(float vol) {
    if (!unka8) {
        mFaderMute->DoFade(vol, 1000.0f);
    }
}

void MetaMusic::Load(float f1, bool b1, bool b2) {
    unk2d = b2;
    unk98 = b1;
    DataArray *cfg = SystemConfig("synth", "metamusic");
    cfg->FindData("fade_time", mFadeTime);
    cfg->FindData("mute_fade_time", mMuteFadeTime);
    cfg->FindData("volume", mVolume);
    mVolume += f1;
    mStartTimes.clear();
    DataArray *startPtsArr = cfg->FindArray("start_points_ms", false);
    if (startPtsArr) {
        for (int i = 1; i < startPtsArr->Size(); i++) {
            mStartTimes.push_back(startPtsArr->Int(i));
        }
    }
    static Symbol song("song");
    DataArray *songArr = cfg->FindArray(song, false);
    mSongInfo = new DataArraySongInfo(songArr, nullptr, "shellmusic");
    if (!unk60.empty()) {
        mShellFx.LoadFile(unk60, true, true, kLoadFront, false);
    }
}

bool MetaMusic::IsStarted() const { return mStarted; }

void MetaMusic::Mute() {
    unka8 = true;
    mFaderMute->DoFade(kDbSilence, mMuteFadeTime * 1000.0f);
    unk30 = 0;
}

void MetaMusic::UnMute() {
    unka8 = false;
    mFaderMute->DoFade(0, mMuteFadeTime * 1000.0f);
    unk30 = 0;
}

bool MetaMusic::Loaded() {
    if (unkb0->IsLoaded() && !unk60.empty()) {
        if (mShellFx.IsLoaded()) {
            if (!mShellFx) {
                mShellFx.PostLoad(nullptr);
            }
            return true;
        }
    }
    return false;
}

bool MetaMusic::IsActive() const { return GetStream() && mFader->IsFading(); }

void MetaMusic::Start() {
    if (Loaded()) {
        Stream *stream = GetStream();
        if (stream && stream->IsPlaying()) {
            mFader->DoFade(mVolume, mFadeTime * 1000.0f);
        } else {
            UnloadStreamFx();
            unk2c = true;
            stream = GetStream();
            stream->Faders()->Add(mFaderMute);
            stream->Faders()->Add(mFader);
            FOREACH (it, mExtraFaders) {
                stream->Faders()->Add(*it);
            }
            if (mShellFx) {
                LoadStreamFx();
                for (int i = 0; i < NumChans(); i++) {
                    stream->SetFXSend(
                        i, mStreamChanFx[i]->Find<FxSendEQ>("eq.send", true)
                    );
                }
            }
            if (unka9) {
                stream->Stop();
                stream->Resync(ChooseStartMs());
            }
            unk30 = 0;
            if (unk2d) {
                stream->SetJump(Stream::kStreamEndMs, 0, nullptr);
            }
            mStarted = true;
            unka9 = true;
        }
    }
}

void MetaMusic::Stop() {
    Stream *stream = GetStream();
    if (stream) {
        if (!stream->IsPlaying()) {
            UnloadStreamFx();
            unk2c = false;
        } else {
            mFader->DoFade(kDbSilence, mFadeTime * 1000.0f);
        }
        mStarted = false;
    }
}

int MetaMusic::NumChans() const {
    const auto *pStream = GetStream();
    MILO_ASSERT(pStream, 268);
    int channel_ct = pStream->GetNumChannels();
    if (channel_ct > 6)
        channel_ct = 6;
    return channel_ct;
}

void MetaMusic::AddFader(Fader *fader) {
    bool found = false;
    FOREACH (it, mExtraFaders) {
        if (*it == fader) {
            found = true;
        }
    }
    if (!found) {
        if (fader) {
            mExtraFaders.push_back(fader);
        } else {
            MILO_NOTIFY("trying to add null fader");
        }
    }
}

void MetaMusic::Poll() {
    Stream *stream = GetStream();
    if (stream) {
        if (!stream->IsPlaying() && stream->IsReady()) {
            mFader->SetVolume(kDbSilence);
            mFader->DoFade(mVolume, mFadeTime * 1000.0f);
            stream->Play();
        }
        if (stream->IsPlaying()) {
            float time = stream->GetTime();
            unkb0->Poll(time);
            if (!mFader->IsFading() && mFader->DuckedValue() == kDbSilence) {
                stream->Stop();
                UnloadStreamFx();
                unk2c = false;
            } else {
                UpdateMix();
            }
            if (!unka8) {
                unk30 += TheTaskMgr.DeltaUISeconds();
            }
        }
    }
}

Stream *MetaMusic::GetStream() const {
    if (unk2c) {
        return unkb0->GetHxAudio()->GetSongStream();
    } else {
        return nullptr;
    }
}

void MetaMusic::LoadStreamFx() {
    MILO_ASSERT(mShellFx, 0x218);
    MILO_ASSERT(mStreamChanFx.empty(), 0x219);
    mStreamChanFx.resize(NumChans());
    for (int i = 0; i < NumChans(); i++) {
        ObjectDir *dir = Hmx::Object::New<ObjectDir>();
        for (ObjDirItr<FxSend> it(mShellFx, true); it != nullptr; ++it) {
            Hmx::Object *cloned = CloneObject(it, false);
            cloned->SetName(it->Name(), dir);
        }
        mStreamChanFx[i] = dir;
    }
}

void MetaMusic::UnloadStreamFx() {
    Stream *stream = GetStream();
    if (stream) {
        for (int i = 0; i < NumChans(); i++) {
            stream->SetFXSend(i, nullptr);
        }
    }
    DeleteAll(mStreamChanFx);
}

int MetaMusic::ChooseStartMs() const {
    int startMs = 0;

    if (mStartTimes.size() != 0) {
        // pick a random element
        int randomInt = RandomInt(0, mStartTimes.size());
        startMs = mStartTimes[randomInt];
    }

    return startMs;
}

void MetaMusic::UpdateMix() {
    Stream *stream = GetStream();
    if (!mShellFx) {
        if (stream && stream->GetNumChannels() == 2) {
            if (unk98) {
                stream->SetPan(0, -2);
                stream->SetPan(1, 2);
            } else {
                stream->SetPan(0, -1);
                stream->SetPan(1, 1);
            }
        }
    } else {
        static Symbol vols("vols");
        static Symbol pans("pans");
        if (unk8c) {
            DataArray *vols8c = unk8c->FindArray(vols);
            DataArray *pans8c = unk8c->FindArray(pans);
            float f16 = ((float)unk94 / 90.0f);
            float f15 = 1.0f - ((float)unk94 / 90.0f);
            if (NumChans() == 2) {
                if (unk90 && unk94 <= 90) {
                    DataArray *vols90 = unk90->FindArray(vols);
                    DataArray *pans90 = unk90->FindArray(pans);
                    for (int i = 0; i < 2; i++) {
                        char buf[16];
                        sprintf(buf, "channel_%d", i + 1);
                        DataArray *buf8c = unk8c->FindArray(buf, false);
                        DataArray *buf90 = unk90->FindArray(buf, false);
                        if (buf8c && buf90) {
                            for (ObjDirItr<FxSend> it(mStreamChanFx[i], true);
                                 it != nullptr;
                                 ++it) {
                                it->EnableUpdates(false);
                                DataArray *thisFxConfigPost =
                                    buf8c->FindArray(it->Name(), false);
                                DataArray *thisFxConfigPre =
                                    buf90->FindArray(it->Name(), false);
                                MILO_ASSERT(thisFxConfigPost, 0x15C);
                                MILO_ASSERT(thisFxConfigPre, 0x15D);
                                MILO_ASSERT(thisFxConfigPre->Size() == thisFxConfigPost->Size(), 0x15E);
                                for (int j = 1; j < thisFxConfigPre->Size(); j++) {
                                    DataArray *preArr = thisFxConfigPre->Array(j);
                                    DataArray *postArr = thisFxConfigPost->Array(j);
                                    float preFloat = preArr->Float(1);
                                    it->SetProperty(
                                        preArr->Sym(0),
                                        postArr->Float(1) * f16 + preFloat * f15
                                    );
                                }
                                it->EnableUpdates(true);
                            }
                        }
                        float volFloat = vols90->Float(i + 1);
                        stream->SetVolume(i, f15 * volFloat + f16 * vols8c->Float(i + 1));
                        float panFloat = pans90->Float(i + 1);
                        stream->SetPan(i, f15 * panFloat + f16 * pans8c->Float(i + 1));
                    }
                } else if (unk94 == 0) {
                    for (int i = 0; i < 2; i++) {
                        char buf[16];
                        sprintf(buf, "channel_%d", i + 1);
                        DataArray *buf8c = unk8c->FindArray(buf, false);
                        if (buf8c) {
                            for (ObjDirItr<FxSend> it(mStreamChanFx[i], true);
                                 it != nullptr;
                                 ++it) {
                                it->EnableUpdates(false);
                                DataArray *thisFxConfigPost =
                                    buf8c->FindArray(it->Name(), false);
                                for (int j = 1; j < thisFxConfigPost->Size(); j++) {
                                    DataArray *postArr = thisFxConfigPost->Array(j);
                                    it->SetProperty(postArr->Sym(0), postArr->Node(1));
                                }
                                it->EnableUpdates(true);
                            }
                        }
                        stream->SetVolume(i, vols8c->Float(i + 1));
                        stream->SetPan(i, pans8c->Float(i + 1));
                    }
                }
                unk94++;
            }
        }
    }
}
