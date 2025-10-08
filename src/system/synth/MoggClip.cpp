#include "synth/MoggClip.h"
#include "MoggClip.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

void MoggClip::Pause(bool pause) {
    mPlaying = !pause;
    if (mStream && !mPlaying) {
        mStream->Stop();
    }
}

bool MoggClip::IsStreaming() const { return mStream && mStream->IsPlaying(); }

void MoggClip::SetVolume(float vol) {
    unk44 = vol;
    if (mStream) {
        mStream->Stream::SetVolume(mVolume + unk44);
    }
}

void MoggClip::ApplyLoop(bool b1, int i2, int i3) {
    if (mStream) {
        mStream->ClearJump();
        if (b1) {
            mStream->SetJumpSamples(i3, i2, 0);
        }
    }
}

void MoggClip::FadeOut(float f1) { unk8c->DoFade(kDbSilence, f1); }

float MoggClip::ElapsedTime() {
    if (!IsStreaming())
        return 0;
    else
        return mStream->GetTime() / 1000;
}

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

const char *MoggClip::GetSoundDisplayName() {
    return !IsPlaying() ? gNullStr
                        : MakeString("MoggClip: %s", FileGetName(mMoggFile.c_str()));
}

BEGIN_COPYS(MoggClip)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(MoggClip)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMoggFile)
        COPY_MEMBER(mVolume)
        COPY_MEMBER(mBufSecs)
    END_COPYING_MEMBERS
END_COPYS

void MoggClip::Stop(bool b1) {
    KillStream();
    if (mUnloadWhenFinished) {
        UnloadData();
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
        for (std::vector<Fader *>::iterator it = mFaders.begin(); it != mFaders.end();
             ++it) {
            mStream->Faders()->Add(*it);
        }
    }
}

void MoggClip::UpdatePanInfo() {
    if (mStream) {
        for (std::vector<PanInfo>::iterator it = mPanInfos.begin(); it != mPanInfos.end();
             ++it) {
            mStream->SetPan(it->channel, it->panning);
        }
    }
}

void MoggClip::LoadNumChannels() {
    if (!mMoggFile.empty()) {
        if (mLoader && !mLoader->IsLoaded()) {
            TheLoadMgr.PollUntilLoaded(mLoader, nullptr);
        }
        SynthPoll();
        if (mStream) {
            for (int i = 0; i < 200; i++) {
                Timer::Sleep(1);
            }
        }
    }
    unk58 = -1;
}

void MoggClip::PostLoad(BinStream &bs) {
    EnsureLoaded();
    LoadNumChannels();
}

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

BEGIN_SAVES(MoggClip)
    SAVE_REVS(3, 2)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMoggFile;
    bs << mVolume;
    bs << mBufSecs;
    bool loading = IsLoadingMusicMogg(mMoggFile.c_str());
    if (bs.Cached() && !loading) {
        FileLoader::SaveData(bs, mData, unk54);
    }
END_SAVES

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

BEGIN_PROPSYNCS(MoggClip)
    SYNC_PROP_SET(file, mMoggFile, SetFile(_val.Str()))
    SYNC_PROP_SET(volume, mVolume, MoggClip::SetVolume(_val.Float()))
    SYNC_PROP(buf_secs, mBufSecs)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

MoggClip::MoggClip() : unk78(this) {}
