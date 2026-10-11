#pragma once
#include "beatmatch/HxAudio.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "synth/Faders.h"
#include "utl/Loader.h"
#include "utl/SongInfoCopy.h"
#include "utl/Symbol.h"

enum CrossfadeState {
    CFS_UNSET = 0x0000,
    CFS_READY = 0x0001,
    CFS_FADE1STHALF = 0x0002,
    CFS_FADE2NDHALF = 0x0003,
};

class HamAudio : public Hmx::Object, public HxAudio {
public:
    struct Crossfade {
        Crossfade() : mXstate(CFS_UNSET) {}
        float mXfromMs;
        float mXtoMs;
        float fadeMs;
        CrossfadeState mXstate;
    };

    HamAudio();
    // Hmx::Object
    virtual ~HamAudio();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // HxAudio
    virtual bool IsReady();
    virtual bool Paused() const;
    virtual void SetPaused(bool);
    virtual void Poll();
    virtual float GetTime() const;
    virtual Stream *GetSongStream() { return mSongStream; }
    virtual void SetMasterVolume(float);

    void SetMuteMaster(bool mute);
    void SetChannelVolume(int, float);
    void SetLoop(float, float);
    void ClearLoop();
    void Jump(float);
    void FinishLoad();
    bool Fail();
    bool IsFinished() const;
    void Load(SongInfo *, bool);
    void Play();
    bool GetCurrLoopMarkers(float &, float &) const;
    bool GetCurrLoopBeats(int &, int &) const;
    void SetCrossfadeJump(float, float, float);

    void SetBackgroundVolume(float);
    void SetForegroundVolume(float);
    void SetStereo(bool);

    DataNode OnGetCurrentLoopBeats(DataArray *);
    DataNode OnSetCrossfadeJump(DataArray *);

private:
    void UpdateMasterFader();
    void Clear();
    void ToggleMuteMaster();
    void PrintFaders();
    void PollCrossfade();
    void DeleteFaders();
    void SetLoop(float, float, Stream *);

    FileLoader *mFileLoader; // 0x30
    void *mAudioData; // 0x34 - file
    int mAudioDataSize; // 0x38 - file size
    SongInfo *mCurrSongInfo; // 0x3c
    Stream *mSongStream; // 0x40
    Stream *mSongStreams[2]; // 0x44
    bool mStreamEnabled; // 0x4c
    Fader *mMasterFader; // 0x50
    float mMasterVolume; // 0x54
    bool mMuteMaster; // 0x58
    bool mInitSend;
    /** See HamAudio::SetCrossfadeJump for usage details */
    Crossfade mQueuedCrossfade; // 0x5c
    /** See HamAudio::SetCrossfadeJump for usage details */
    Crossfade mActiveCrossfade;
    Fader *mCrossFaders[2]; // 0x7c
    std::vector<Fader *> mChannels; // 0x84
    std::map<Symbol, Fader *> mTracks; // 0x90
};
