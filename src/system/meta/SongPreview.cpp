#include "meta/SongPreview.h"
#include "SongMetadata.h"
#include "SongMgr.h"
#include "macros.h"
#include "math/Utl.h"
#include "meta/DataArraySongInfo.h"
#include "movie/TexMovie.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "synth/Faders.h"
#include "synth/Synth.h"
#include "utl/SongInfoCopy.h"
#include "utl/Symbol.h"

const float SongPreview::kSilenceVal = -48;

#pragma region Hmx::Object

SongPreview::SongPreview(const SongMgr &mgr)
    : mSongMgr(mgr), mStream(0), mTexMovie(this), unk4c(0), mFader(0), mMusicFader(0),
      mCrowdSingFader(0), mNumChannels(0), mAttenuation(0.0f), mPreviewDb(0.0f),
      mState(kIdle), mStartMs(0.0f), mEndMs(0.0f), mStartPreviewMs(0.0f),
      mEndPreviewMs(0.0f), mRegisteredWithCM(0), unk8d(0), mSecurePreview(0) {}

SongPreview::~SongPreview() { Terminate(); }

BEGIN_HANDLERS(SongPreview)
    HANDLE(start, OnStart)
    HANDLE_ACTION(start_video, Start(_msg->Sym(2), _msg->Obj<TexMovie>(3)))
    HANDLE_ACTION(set_music_vol, SetMusicVol(_msg->Float(2)))
    HANDLE_ACTION(set_crowd_sing_vol, SetCrowdSingVol(_msg->Float(2)))
    HANDLE_EXPR(get_song, mSong)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion
#pragma region ContentMgr::Callback

void SongPreview::ContentMounted(const char *contentName, const char *) {
    MILO_ASSERT(contentName, 0xbf);
    Symbol s = contentName;
    if (s == mSongContent) {
        mSongContent = 0;
    }
}

void SongPreview::ContentFailed(const char *contentName) {
    MILO_ASSERT(contentName, 0xcb);
    // it matches, don't question it
    Symbol sym = contentName;
    if (sym == mSongContent) {
        mSong = 0;
        Symbol zero = 0;
        mState = kIdle;
        mSongContent = zero;
    }
}

#pragma endregion
#pragma region SongPreview

bool SongPreview::IsWaitingToDelete() const { return mState == kDeletingSong; }
bool SongPreview::IsFadingOut() const { return mState == kFadingOutSong; }

void SongPreview::SetMusicVol(float f) {
    if (unk4c == 0) {
        return;
    }
    if (f < mMusicFader->GetLevelTarget()) {
        mMusicFader->DoFade(f, 250.0f);
    } else {
        mMusicFader->DoFade(f, 1000.0f);
    }
}

void SongPreview::SetCrowdSingVol(float f) {
    if (unk4c)
        mCrowdSingFader->DoFade(f, 0.0f);
}

void SongPreview::Init() {
    if (!unk4c) {
        unk4c = true;
        mSong = 0;
        mSongContent = 0;
        RELEASE(mStream);
        mTexMovie = nullptr;
        mState = kIdle;
        mRestart = true;
        DataArray *cfg = SystemConfig("sound", "song_select");
        cfg->FindData("loop_forever", mLoopForever, true);
        cfg->FindData("fade_time", mFadeTime, true);
        cfg->FindData("attenuation", mAttenuation, true);
        cfg->FindData("preview_db", mPreviewDb, true);
        mFadeTime *= 1000.0f;
        mFader = Hmx::Object::New<Fader>();
        mMusicFader = Hmx::Object::New<Fader>();
        mCrowdSingFader = Hmx::Object::New<Fader>();
        mCrowdSingFader->SetVolume(kDbSilence);
    }
}

void SongPreview::Terminate() {
    if (unk4c) {
        unk4c = 0;
        DetachFader(mMusicFader);
        DetachFader(mCrowdSingFader);
        mSong = 0;
        mSongContent = 0;
        RELEASE(mStream);
        RELEASE(mFader);
        RELEASE(mMusicFader);
        RELEASE(mCrowdSingFader);
        mTexMovie = nullptr;
        if (mRegisteredWithCM) {
            TheContentMgr.UnregisterCallback(this, true);
            mRegisteredWithCM = 0;
        }
    }
}

void SongPreview::Start(Symbol song, TexMovie *texMovie) {
    if (unk4c || !song.Null()) {
        MILO_ASSERT(mFader && mMusicFader && mCrowdSingFader,0x6c);
        mTexMovie = texMovie;
        if (song == mSong) {
            unk8d = true;
        } else {
            if (!song.Null()) {
                if (!mSongMgr.HasSong(song, false)) {
                    return;
                }
                int songID = mSongMgr.GetSongIDFromShortName(song, true);
                const SongMetadata *data = mSongMgr.Data(songID);
                if (data && !data->IsVersionOK()) {
                    song = gNullStr;
                }
                if (!mRegisteredWithCM) {
                    TheContentMgr.RegisterCallback(this, false);
                    mRegisteredWithCM = true;
                }
            }
            mSong = song;
            mRestart = true;
            mMusicFader->SetVolume(mPreviewDb);
            mCrowdSingFader->SetVolume(kDbSilence);
            switch (mState) {
            case kIdle:
            case kMountingSong:
                RELEASE(mStream);
                mState = kIdle;
                break;
            case kPreparingSong:
                mState = kDeletingSong;
                break;
            case kPlayingSong:
                if (!song.Null()) {
                    mFader->DoFade(kSilenceVal, mFadeTime);
                    mState = kFadingOutSong;
                } else {
                    mFader->DoFade(kSilenceVal, 0);
                    mState = kIdle;
                }
                break;
            default:
                break;
            }
        }
    }
}

void SongPreview::PreparePreview() {
    float previewstart = 0.0f;
    float previewend = 15000.0f;
    if (mStartPreviewMs != 0 || mEndPreviewMs != 0) {
        previewstart = mStartPreviewMs;
        previewend = mEndPreviewMs;
    } else {
        int songid = mSongMgr.GetSongIDFromShortName(mSong);
        const SongMetadata *data = mSongMgr.Data(songid);
        data->PreviewTimes(previewstart, previewend);
    }
    mStartMs = previewstart;
    mEndMs = previewend;
    PrepareSong(mSong);
    if (!mLoopForever)
        mRestart = false;
}

DataNode SongPreview::OnStart(DataArray *arr) {
    mSecurePreview = false;
    if (arr->Size() == 3) {
        mStartPreviewMs = 0;
        mEndPreviewMs = 0;
        MILO_LOG(
            "start called in upper OnStart here : sym='%s'\n", arr->ForceSym(2).Str()
        );
        Start(arr->ForceSym(2), nullptr);
    } else {
        mStartPreviewMs = arr->Float(3);
        mEndPreviewMs = arr->Float(4);
        if (arr->Size() >= 6) {
            mSecurePreview = arr->Int(5);
        }
        mSong = gNullStr;
        MILO_LOG(
            "start called in lower OnStart here : sym='%s'\n", arr->ForceSym(2).Str()
        );
        Start(arr->ForceSym(2), nullptr);
    }
    return 1;
}

void SongPreview::DetachFader(Fader *f) {
    if (mStream && f) {
        for (int i = 0; i < mNumChannels; i++) {
            mStream->ChannelFaders(i).Remove(f);
        }
    }
}

void SongPreview::PrepareFaders(const SongInfo *info) {
    for (int i = 0; i < mNumChannels; i++) {
        FaderGroup &f = mStream->ChannelFaders(i);
        f.Add(mMusicFader);
    }
}

void SongPreview::PrepareSong(Symbol song) {
    mState = kPreparingSong;
    RELEASE(mStream);
    DataArraySongInfo songInfo(mSongMgr.SongAudioData(song));
    const char *filename = songInfo.GetBaseFileName();
    if (mTexMovie) {
        String str(mSongMgr.SongFilePath(song, "_prev.bik", 10));
        if (FileExists(str.c_str(), 0, nullptr)) {
            mTexMovie->SetFile(str.c_str());
            mTexMovie->SetVolume(-mAttenuation);
            mTexMovie->AddFader(mFader);
            mTexMovie->AddFader(mMusicFader);
            return;
        }
        mTexMovie->SetFile(gNullStr);
    }
    mStream = TheSynth->NewStream(filename, mStartMs, 0, mSecurePreview);
    const std::vector<float> &pans = songInfo.GetPans();
    const std::vector<float> &vols = songInfo.GetVols();
    mNumChannels = pans.size();
    for (int i = 0; i < mNumChannels; i++) {
        mStream->SetVolume(i, vols[i]);
        mStream->SetPan(i, pans[i]);
    }
    const TrackChannels *trackChannels = songInfo.FindTrackChannel(kAudioTypeMulti);
    if (trackChannels) {
        for (int i = 0; i < trackChannels->mChannels.size(); i++) {
            mStream->SetVolume(trackChannels->mChannels[i], kDbSilence);
        }
    }
    DetachFader(mMusicFader);
    DetachFader(mCrowdSingFader);
    PrepareFaders(&songInfo);
    mStream->SetVolume(-mAttenuation);
    mStream->Faders()->Add(mFader);
}

void SongPreview::Poll() {
    switch (mState) {
    case kIdle: {
        if (!mSong.Null() && mRestart) {
            const char *name = mSongMgr.ContentName(mSong);
            if (name) {
                mSongContent = name;
                if (TheContentMgr.MountContent(mSongContent.Str())) {
                    mSongContent = 0;
                }
                mState = kMountingSong;
            } else {
                PreparePreview();
            }
        } else if (unk8d) {
            mState = kFadingOutSong;
            mFader->DoFade(kSilenceVal, mFadeTime);
            unk8d = false;
        }
        break;
    }
    case kMountingSong: {
        if (mSongContent.Null()) {
            PreparePreview();
        }
        break;
    }
    case kPreparingSong: {
        if (HasMovie()) {
            mFader->SetVolume(0);
        } else {
            if (!mStream->IsReady()) {
                return;
            }
            mFader->SetVolume(kSilenceVal);
            mFader->DoFade(0, mFadeTime);
            mStream->Play();
        }
        mState = kPlayingSong;
        break;
    }
    case kDeletingSong: {
        RELEASE(mStream);
        mState = kIdle;
        break;
    }
    case kPlayingSong: {
        if (HasMovie() || mStream && mStream->GetTime() < mEndMs)
            return;
        MILO_LOG("mSong in Poll is %s\n", mSong);
        mState = kFadingOutSong;
        mFader->DoFade(kSilenceVal, mFadeTime);
        break;
    }
    case kFadingOutSong: {
        if (!mFader->IsFading()) {
            RELEASE(mStream);
            if (HasMovie()) {
                mTexMovie->SetFile(gNullStr);
            }
            mState = kIdle;
        }
        break;
    }
    default:
        break;
    }
}
