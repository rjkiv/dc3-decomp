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
    : mVolume(0), unk44(0), mStream(nullptr), unk4c(0), mData(nullptr), unk54(0),
      mLoader(nullptr), unk78(this), mFader(Hmx::Object::New<Fader>()),
      mUnloadWhenFinished(false), mPlaying(false), mLoop(false), unk94(0), unk98(-1),
      mBufSecs(0) {
    mFaders.push_back(mFader);
    StartPolling();
}

MoggClip::~MoggClip() {
    RELEASE(mLoader);
    RELEASE(mFader);
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
    SYNC_PROP_SET(file, mMoggFile, SetFile(_val.Str()))
    SYNC_PROP_SET(volume, mVolume, MoggClip::SetVolume(_val.Float()))
    SYNC_PROP(buf_secs, mBufSecs)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(MoggClip)
    SAVE_REVS(3, 2)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMoggFile << mVolume;
    bs << mBufSecs;
    bool loading = IsLoadingMusicMogg(mMoggFile.c_str());
    if (bs.Cached() && !loading) {
        FileLoader::SaveData(bs, mData, unk54);
    }
END_SAVES

BEGIN_COPYS(MoggClip)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(MoggClip)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMoggFile)
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
    Hmx::Object::Load(bs);
    bs >> mMoggFile;
    bs >> mVolume;
    if (d.rev <= 2) {
        bool b60;
        d >> b60;
        if (d.rev > 1) {
            int x, y;
            bs >> x >> y;
        }
    }
    if (d.altRev > 1) {
        bs >> mBufSecs;
    }
    LoadFile(d.rev > 0 ? &bs : 0);
    if (d.altRev == 1) {
        bs >> mBufSecs;
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
                        : MakeString("MoggClip: %s", FileGetName(mMoggFile.c_str()));
}

void MoggClip::SynthPoll() {
    if (mPlaying && mStream) {
        mStream->PollStream();
        if (!mStream->IsPlaying() && mStream->IsReady()) {
            if (mPanInfos.empty()) {
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
            if (mStream->IsFinished() || mFader->DuckedValue() == kDbSilence)
                Stop(0);
        }
    }
}

#pragma endregion
#pragma region PlayableSample

void MoggClip::Play(float f1) {
    if (EnsureLoaded()) {
        KillStream();
        Stream *stream = TheSynth->NewBufStream(mData, unk54, "mogg", 0, false);
        mStream = dynamic_cast<StandardStream *>(stream);
        if (mBufSecs > 0) {
            // set mStream + 0x30 = mBufSecs
        }
        if (!mStream) {
            delete stream;
        } else {
            mFader->SetVolume(0);
            SetVolume(f1);
            MoggClip::SetVolume(mVolume);
            UpdateFaders();
            UpdatePanInfo();
            ApplyLoop(mLoop, unk94, unk98);
            for (int i = 0; i < mStream->GetNumChanParams(); i++) {
                mStream->SetFXSend(i, unk78);
            }
            mPlaying = true;
        }
    } else
        MILO_NOTIFY("Mogg file not loaded: '%s'", mMoggFile.c_str());
}

void MoggClip::Stop(bool b1) {
    KillStream();
    if (mUnloadWhenFinished) {
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
    unk44 = vol;
    if (mStream) {
        mStream->Stream::SetVolume(mVolume + unk44);
    }
}

void MoggClip::SetPan(float f1) {
    if (unk58 == 1) {
        SetPan(0, f1);
    }
}

void MoggClip::SetSend(FxSend *send) { unk78 = send; }

void MoggClip::EndLoop() { SetLoop(false, unk94, unk98); }

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

void MoggClip::FadeOut(float f1) { mFader->DoFade(kDbSilence, f1); }

void MoggClip::UnloadWhenFinishedPlaying(bool unload) { mUnloadWhenFinished = unload; }

bool MoggClip::IsReadyToPlay() const {
    if (mLoader)
        return mLoader->IsLoaded();
    else
        return mData && unk54 > 0;
}

void MoggClip::KillStream() {
    mPlaying = false;
    RELEASE(mStream);
}

void MoggClip::UnloadData() {
    if (mData) {
        MemFree(mData);
        mData = nullptr;
        unk54 = 0;
    }
}

void MoggClip::SetLoop(bool b1, int i2, int i3) {
    mLoop = b1;
    unk94 = i2;
    unk98 = i3;
    ApplyLoop(mLoop, unk94, unk98);
}

bool MoggClip::EnsureLoaded() {
    if (mLoader) {
        if (!mLoader->IsLoaded()) {
            MILO_NOTIFY("MoggClip blocked while loading '%s'", mMoggFile.c_str());
            TheLoadMgr.PollUntilLoaded(mLoader, nullptr);
        }
        mData = mLoader->GetBuffer(&unk54);
        RELEASE(mLoader);
    }
    return mData && unk54 > 0;
}

void MoggClip::UpdateFaders() {
    if (mStream) {
        FOREACH (it, mFaders) {
            mStream->Faders()->Add(*it);
        }
    }
}

void MoggClip::UpdatePanInfo() {
    if (mStream) {
        FOREACH (it, mPanInfos) {
            mStream->SetPan(it->channel, it->panning);
        }
    }
}

void MoggClip::LoadNumChannels() {
    if (mMoggFile.empty()) {
        unk58 = -1;
    } else {
        if (mLoader && !mLoader->IsLoaded()) {
            TheLoadMgr.PollUntilLoaded(mLoader, nullptr);
        }
        SynthPoll();
        if (mStream) {
            for (int i = 0; i < 200; i++) {
                Timer::Sleep(1);
                TheSynth->Poll();
            }
        }
    }
}

void MoggClip::LoadFile(BinStream *bs) {
    RELEASE(mLoader);
    KillStream();
    UnloadData();
    unk58 = -1;
    if (!mMoggFile.empty()) {
        bool loadingMusic = IsLoadingMusicMogg(mMoggFile.c_str());
        bool useless = IsUselessMogg(mMoggFile.c_str());
        if (useless) {
            if (!bs || !bs->Cached() || loadingMusic)
                bs = nullptr;
            mLoader = new FileLoader(
                mMoggFile,
                FileLocalize(mMoggFile.c_str(), nullptr),
                kLoadFront,
                0,
                false,
                true,
                bs,
                0
            );
            if (!mLoader) {
                MILO_NOTIFY("Could not load mogg file '%s'", mMoggFile.c_str());
            }
        } else {
            MILO_ASSERT(!mLoader && !mData, 0x23C);
        }
    }
}

void MoggClip::SetFile(const char *file) {
    MILO_ASSERT(file != NULL, 0x14C);
    mMoggFile.Set(FilePath::Root().c_str(), file);
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
            mStream->Faders()->Add(fader);
        }
    }
}
