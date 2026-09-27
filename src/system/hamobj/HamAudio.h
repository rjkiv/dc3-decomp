#pragma once
#include "beatmatch/HxAudio.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "synth/Faders.h"
#include "utl/Loader.h"
#include "utl/SongInfoCopy.h"
#include "utl/Symbol.h"

class HamAudio : public Hmx::Object, public HxAudio {
public:
    struct Crossfade {
        Crossfade() : unkc(0) {}
        float unk0;
        float unk4;
        float unk8;
        int unkc;
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
    char *unk34;
    int unk38;
    SongInfo *mSongInfo; // 0x3c
    Stream *mSongStream; // 0x40
    Stream *unk44[2]; // 0x44
    bool mReady; // 0x4c
    Fader *mMasterFader; // 0x50
    float mMasterVolume; // 0x54
    bool mMuteMaster; // 0x58
    bool unk59;
    Crossfade unk5c; // 0x5c
    Crossfade unk6c;
    Fader *mCrossFaders[2]; // 0x7c
    std::vector<Fader *> mChannels; // 0x84
    std::map<Symbol, Fader *> mTracks; // 0x90
};
