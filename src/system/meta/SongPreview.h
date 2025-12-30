#pragma once
#include "meta/SongMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "movie/TexMovie.h"
#include "synth/Faders.h"
#include "utl/Symbol.h"
#include "synth/Stream.h"

class SongPreview : public ContentMgr::Callback, public Hmx::Object {
public:
    enum State {
        kIdle = 0,
        kMountingSong = 1,
        kPreparingSong = 2,
        kDeletingSong = 3,
        kPlayingSong = 4,
        kFadingOutSong = 5,
    };

    SongPreview(const SongMgr &);
    // ContentMgr::Callback
    virtual ~SongPreview();
    virtual void ContentMounted(const char *, const char *);
    virtual void ContentFailed(const char *);

    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    bool IsWaitingToDelete() const;
    bool IsFadingOut() const;
    void SetMusicVol(float);
    void Init();
    void Terminate();
    void Start(Symbol, TexMovie *);
    void PreparePreview();
    void Poll();
    DataNode OnStart(DataArray *);
    void SetCrowdSingVol(float);

    bool HasMovie() const { return mTexMovie && !mTexMovie->IsEmpty(); }
    float PreviewDb() const { return mPreviewDb; }

    static const float kSilenceVal;

private:
    const SongMgr &mSongMgr; // 0x30
    Stream *mStream; // 0x34
    ObjPtr<TexMovie> mTexMovie; // 0x38
    bool unk4c; // 0x4c - initted?
    Fader *mFader; // 0x50
    Fader *mMusicFader; // 0x54
    Fader *mCrowdSingFader; // 0x58
    int mNumChannels; // 0x5c
    float mAttenuation; // 0x60
    float mFadeTime; // 0x64
    float mPreviewDb; // 0x68
    bool mRestart; // 0x6c
    bool mLoopForever; // 0x6d
    bool unk6e; // 0x6e
    bool unk6f; // 0x6f
    State mState; // 0x70
    Symbol mSong; // 0x74
    Symbol mSongContent; // 0x78
    float mStartMs; // 0x7c
    float mEndMs; // 0x80
    float mStartPreviewMs; // 0x84
    float mEndPreviewMs; // 0x88
    bool mRegisteredWithCM; // 0x8c
    bool unk8d; // 0x8d - preview currently playing?
    bool mSecurePreview; // 0x8e

    void DetachFader(Fader *);
    void PrepareFaders(const SongInfo *);
    void PrepareSong(Symbol);
};
