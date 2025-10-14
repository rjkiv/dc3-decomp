#pragma once
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

    // ContentMgr::Callback
    virtual ~SongPreview();
    virtual void ContentMounted(char const *, char const *);
    virtual void ContentFailed(char const *);

    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    // unsure about these, will wait until TexMovie is filled out
    // virtual ObjPtr<class TexMovie>::ObjPtr<class TexMovie>(void);
    // virtual void ObjRefConcrete<class TexMovie, class ObjectDir>::SetObj();

    SongPreview(class SongMgr const &);
    bool IsWaitingToDelete() const;
    bool IsFadingOut() const;
    void SetMusicVol(float);
    void Init();
    void Terminate();
    void Start(class Symbol, class TexMovie *);
    void PreparePreview();
    void Poll();
    DataNode OnStart(DataArray *);

    const SongMgr &mSongMgr; // 0x30
    Stream *unk34;
    ObjPtr<TexMovie> unk38;
    bool unk4c;
    Fader *mFader; // 0x50
    Fader *mMusicFader; // 0x54
    Fader *mCrowdSingFader; // 0x58
    int unk5c;
    float unk60;
    float unk64;
    float unk68;
    bool unk6c;
    bool unk6d;
    bool unk6e;
    bool unk6f;
    int unk70;
    Symbol unk74;
    Symbol unk78;
    float unk7c;
    float unk80;
    float unk84;
    float unk88;
    bool unk8c;
    bool unk8d;
    bool unk8e;

private:
    void DetachFader(Fader *);
    void PrepareFaders(class SongInfo const *);
    void PrepareSong(Symbol);
};
