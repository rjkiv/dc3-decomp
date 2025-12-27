#pragma once
#include "beatmatch/HxMaster.h"
#include "meta/DataArraySongInfo.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "synth/Faders.h"
#include "synth/Stream.h"
#include "math/Rand.h"
#include "meta/MetaMusicScene.h"
#include "utl/MemMgr.h"
#include "utl/Loader.h"

class MetaMusic : public Hmx::Object {
public:
    MetaMusic(HxMaster *, const char *);
    virtual ~MetaMusic();
    virtual DataNode Handle(DataArray *, bool);

    bool IsFading() const;
    bool IsPlaying() const;
    bool Loaded();
    void Mute();
    void UnMute();
    void Stop();
    void Start();
    void AddFader(Fader *);
    void Load(float, bool, bool);
    void Poll();
    bool IsActive() const;
    void SetQuietVolume(float);
    bool IsStarted() const;

    // float SomeMinusFunc() { return 1.0f - (float)unk84 / 90.0f; }
    // float SomePlusFunc() { return (float)unk84 / 90.0f; }
    DataArraySongInfo *SongInfo() { return mSongInfo; }

private:
    Stream *GetStream() const;
    int NumChans() const;
    int ChooseStartMs() const;
    void LoadStreamFx();
    void UnloadStreamFx();
    void UpdateMix();

    bool unk2c; // 0x2c
    bool unk2d; // 0x2d - loop?
    float unk30; // 0x30
    float mFadeTime; // 0x34
    float mMuteFadeTime; // 0x38
    float mVolume; // 0x3c
    Symbol unk40; // 0x40
    Fader *mFader; // 0x44
    Fader *mFaderMute; // 0x48
    ObjPtrList<Fader> mExtraFaders; // 0x4c
    FilePath unk60; // 0x60
    ObjDirPtr<ObjectDir> mShellFx; // 0x68
    std::vector<ObjectDir *> mStreamChanFx; // 0x7c
    bool mStarted; // 0x88
    DataArray *unk8c; // 0x8c - pre?
    DataArray *unk90; // 0x90 - post?
    int unk94; // 0x94
    bool unk98; // 0x98
    std::vector<int> mStartTimes; // 0x9c
    bool unka8; // 0xa8
    bool unka9; // 0xa9
    DataArraySongInfo *mSongInfo; // 0xac
    HxMaster *unkb0; // 0xb0
};

extern MetaMusic *TheMetaMusic;
