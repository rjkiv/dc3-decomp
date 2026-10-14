#include "synth/MoggClip.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "synth/FxSend.h"
#include "synth/Stream.h"
#include "synth/Synth.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

bool IsLoadingMusicMogg(const char *mogg) {
    static Symbol is_loading_music_mogg("is_loading_music_mogg");
    static DataArrayPtr func(new DataArray(2));
    func->Node(0) = is_loading_music_mogg;
    func->Node(1) = mogg;
    DataNode exec = func->Execute(false);
    return exec.Int();
}

bool IsUselessMogg(const char *mogg) {
    static Symbol is_useless_mogg_load("is_useless_mogg_load");
    static DataArrayPtr func(new DataArray(2));
    func->Node(0) = is_useless_mogg_load;
    func->Node(1) = mogg;
    DataNode exec = func->Execute(false);
    return exec.Int();
}

#pragma region Hmx::Object

MoggClip::MoggClip()
    : mVolume(0), mControllerVolume(0), mStream(nullptr), mPlaybackVolumeOffset(0),
      mData(nullptr), mSize(0), mLoader(nullptr), mSend(this),
      mFadeOutFader(Hmx::Object::New<Fader>()), mUnloadWhenFinishedPlaying(false),
      mPlaying(false), mLoop(false), mLoopStartSamp(0), mLoopEndSamp(-1), mBufSecs(0) {
    mFaders.push_back(mFadeOutFader);
    StartPolling();
}

MoggClip::~MoggClip() {
    RELEASE(mLoader);
    RELEASE(mFadeOutFader);
    KillStream();
    UnloadData();
}

BEGIN_HANDLERS(MoggClip)
    HANDLE_ACTION(play, Play(0))
    HANDLE_ACTION(stop, Stop(0))
    HANDLE_ACTION(set_pan, SetPan(_msg->Int(2), _msg->Float(3)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(MoggClip)
    SYNC_PROP_SET(file, mFile, SetFile(_val.Str()))
    SYNC_PROP_SET(volume, mVolume, SetMoggVolume(_val.Float()))
    SYNC_PROP(buf_secs, mBufSecs)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(MoggClip)
    SAVE_REVS(3, 2)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mFile << mVolume;
    bs << mBufSecs;
    bool loading = IsLoadingMusicMogg(mFile.c_str());
    if (bs.Cached() && !loading) {
        FileLoader::SaveData(bs, mData, mSize);
    }
END_SAVES

BEGIN_COPYS(MoggClip)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(MoggClip)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mFile)
        COPY_MEMBER(mVolume)
        COPY_MEMBER(mBufSecs)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(MoggClip)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(3, 2)

void MoggClip::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(3, 2)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mFile;
    d >> mVolume;
    if (d.rev <= 2) {
        bool b60;
        d >> b60;
        if (d.rev > 1) {
            int x, y;
            d >> x >> y;
        }
    }
    if (d.altRev > 1) {
        d >> mBufSecs;
    }
    LoadFile(d.rev > 0 ? &bs : 0);
    if (d.altRev == 1) {
        d >> mBufSecs;
    }
}

void MoggClip::PostLoad(BinStream &bs) {
    EnsureLoaded();
    LoadNumChannels();
}

#pragma endregion
#pragma region SynthPollable

const char *MoggClip::GetSoundDisplayName() {
    return !IsPlaying() ? gNullStr
                        : MakeString("MoggClip: %s", FileGetName(mFile.c_str()));
}

void MoggClip::SynthPoll() {
    if (mPlaying && mStream) {
        mStream->PollStream();
        if (!mStream->IsPlaying() && mStream->IsReady()) {
            if (mPanInfo.empty()) {
                int chans = mStream->GetNumChannels();
                if (chans == 1) {
                    mStream->SetPan(0, 0);
                } else if (chans == 2) {
                    mStream->SetPan(0, -1);
                    mStream->SetPan(1, 1);
                }
            }
            mStream->Play();
            static Message msg("mogg_ready");
            Export(msg, true);
        } else {
            if (mStream->IsFinished() || mFadeOutFader->DuckedValue() == kDbSilence)
                Stop(0);
        }
    }
}

#pragma endregion
#pragma region PlayableSample

void MoggClip::Play(float f1) {
    if (EnsureLoaded()) {
        KillStream();
        Stream *stream = TheSynth->NewBufStream(mData, mSize, "mogg", 0, false);
        mStream = dynamic_cast<StandardStream *>(stream);
        if (mBufSecs > 0) {
            mStream->SetBufSecs(mBufSecs);
        }
        if (!mStream) {
            delete stream;
        } else {
            mFadeOutFader->SetVolume(0);
            SetVolume(f1);
            SetMoggVolume(mVolume);
            UpdateFaders();
            UpdatePanInfo();
            ApplyLoop(mLoop, mLoopStartSamp, mLoopEndSamp);
            for (int i = 0; i < mStream->GetNumChanParams(); i++) {
                mStream->SetFXSend(i, mSend);
            }
            mPlaying = true;
        }
    } else
        MILO_NOTIFY("Mogg file not loaded: '%s'", mFile.c_str());
}

void MoggClip::Stop(bool b1) {
    KillStream();
    if (mUnloadWhenFinishedPlaying) {
        UnloadData();
    }
}

void MoggClip::Pause(bool pause) {
    mPlaying = !pause;
    if (mStream && !mPlaying) {
        mStream->Stop();
    }
}

bool MoggClip::DonePlaying() { return !mStream; }

void MoggClip::SetVolume(float vol) {
    mControllerVolume = vol;
    if (mStream) {
        mStream->Stream::SetVolume(mVolume + mControllerVolume);
    }
}

void MoggClip::SetPan(float f1) {
    if (mNumChannels == 1) {
        SetPan(0, f1);
    }
}

void MoggClip::SetSend(FxSend *send) { mSend = send; }

void MoggClip::EndLoop() { SetLoop(false, mLoopStartSamp, mLoopEndSamp); }

float MoggClip::ElapsedTime() {
    if (!IsStreaming())
        return 0;
    else
        return mStream->GetTime() / 1000;
}

#pragma endregion
#pragma region MoggClip

bool MoggClip::IsStreaming() const { return mStream && mStream->IsPlaying(); }

void MoggClip::ApplyLoop(bool b1, int i2, int i3) {
    if (mStream) {
        mStream->ClearJump();
        if (b1) {
            mStream->SetJumpSamples(i3, i2, 0);
        }
    }
}

void MoggClip::FadeOut(float f1) { mFadeOutFader->DoFade(kDbSilence, f1); }

void MoggClip::UnloadWhenFinishedPlaying(bool unload) {
    mUnloadWhenFinishedPlaying = unload;
}

bool MoggClip::IsReadyToPlay() const {
    if (mLoader)
        return mLoader->IsLoaded();
    else
        return mData && mSize > 0;
}

void MoggClip::KillStream() {
    mPlaying = false;
    RELEASE(mStream);
}

void MoggClip::UnloadData() {
    if (mData) {
        MemFree(mData);
        mData = nullptr;
        mSize = 0;
    }
}

void MoggClip::SetLoop(bool b1, int i2, int i3) {
    mLoop = b1;
    mLoopStartSamp = i2;
    mLoopEndSamp = i3;
    ApplyLoop(mLoop, mLoopStartSamp, mLoopEndSamp);
}

bool MoggClip::EnsureLoaded() {
    if (mLoader) {
        if (!mLoader->IsLoaded()) {
            MILO_NOTIFY("MoggClip blocked while loading '%s'", mFile.c_str());
            TheLoadMgr.PollUntilLoaded(mLoader, nullptr);
        }
        mData = mLoader->GetBuffer(&mSize);
        RELEASE(mLoader);
    }
    return mData && mSize > 0;
}

void MoggClip::UpdateFaders() {
    if (mStream) {
        FOREACH (it, mFaders) {
            mStream->Faders().Add(*it);
        }
    }
}

void MoggClip::UpdatePanInfo() {
    if (mStream) {
        FOREACH (it, mPanInfo) {
            mStream->SetPan(it->mChannel, it->mPan);
        }
    }
}

void MoggClip::LoadNumChannels() {
    if (mFile.empty()) {
        mNumChannels = -1;
        return;
    }
    if (mLoader && !mLoader->IsLoaded()) {
        TheLoadMgr.PollUntilLoaded(mLoader, nullptr);
    }
    Play(0);
    if (!mStream) {
        mNumChannels = -1;
        return;
    }
    for (int i = 0; i < 200; i++) {
        Timer::Sleep(1);
        TheSynth->Poll();
        if (mStream->GetNumInfoChannels() > 0)
            break;
    }
    mNumChannels = mStream->GetNumInfoChannels();
    Stop(false);
    if (mNumChannels < 0) {
        MILO_NOTIFY(
            "[GetNumChannels] Ret = %d.  Unable to get the number of channels from mogg: %s!",
            mNumChannels,
            mFile
        );
        mNumChannels = -1;
    }
}

void MoggClip::LoadFile(BinStream *bs) {
    RELEASE(mLoader);
    KillStream();
    UnloadData();
    mNumChannels = -1;
    if (!mFile.empty()) {
        bool loadingMusic = IsLoadingMusicMogg(mFile.c_str());
        if (!IsUselessMogg(mFile.c_str())) {
            BinStream *bsToUse = (bs && bs->Cached() && !loadingMusic) ? bs : nullptr;
            mLoader = new FileLoader(
                mFile,
                FileLocalize(mFile.c_str(), nullptr),
                kLoadFront,
                0,
                false,
                true,
                bsToUse,
                0
            );
            if (!mLoader) {
                MILO_NOTIFY("Could not load mogg file '%s'", mFile.c_str());
            }
        } else {
            MILO_ASSERT(!mLoader && !mData, 0x23C);
        }
    }
}

void MoggClip::SetFile(const char *file) {
    MILO_ASSERT(file != NULL, 0x14C);
    mFile.Set(FilePath::Root().c_str(), file);
    LoadFile(nullptr);
    LoadNumChannels();
}

void MoggClip::AddFader(Fader *fader) {
    if (fader) {
        bool b1 = false;
        FOREACH (it, mFaders) {
            if (*it == fader) {
                b1 = true;
                break;
            }
        }
        if (!b1) {
            mFaders.push_back(fader);
        }
        if (mStream) {
            mStream->Faders().Add(fader);
        }
    }
}

void MoggClip::SetPan(int i1, float f2) {
    bool found = false;
    PanInfo info;
    info.mChannel = i1;
    info.mPan = f2;
    FOREACH (it, mPanInfo) {
        if (it->mChannel == i1) {
            found = true;
            *it = info;
            break;
        }
    }
    if (!found) {
        mPanInfo.push_back(info);
    }
    if (mStream) {
        mStream->SetPan(i1, f2);
    }
}

void MoggClip::SetMoggVolume(float vol) {
    mVolume = vol;
    if (mStream) {
        mStream->Stream::SetVolume(mVolume + mControllerVolume);
    }
}

void MoggClip::SetupPanInfo(float f1, float f2, bool stereo) {
    if (stereo) {
        SetPan(0, (f2 / -2) + f1);
        SetPan(1, (f2 / 2) + f1);
    } else {
        SetPan(0, f1);
    }
}
