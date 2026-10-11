#pragma once
#include "hamobj/HamAudio.h"
#include "beatmatch/HxMaster.h"
#include "hamobj/HamSongData.h"
#include "math/Vec.h"
#include "midi/Midi.h"
#include "midi/MidiParserMgr.h"
#include "obj/Object.h"
#include "utl/Loader.h"
#include "utl/SongInfoCopy.h"
#include "utl/SongPos.h"

class HamMaster;

class HamMasterLoader : public Loader {
public:
    HamMasterLoader(HamMaster *);
    virtual ~HamMasterLoader() {}
    virtual const char *DebugText() { return "HamMasterLoader"; }
    virtual bool IsLoaded() const { return false; }

protected:
    virtual void PollLoading();

    HamMaster *mMaster; // 0x1c
};

class HamMaster : public Hmx::Object, public HxMaster {
public:
    HamMaster(HamSongData *, MidiParserMgr *);
    // Hmx::Object
    virtual ~HamMaster();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // HxMaster
    virtual void Poll(float);
    virtual void Jump(float);
    virtual void Reset();
    virtual HxAudio *GetHxAudio() { return mAudio ? mAudio : nullptr; }
    virtual float SongDurationMs();
    virtual bool IsLoaded() { return mLoaded; }

    void
    Load(SongInfo *, bool, int, bool, HamSongDataValidate, std::vector<MidiReceiver *> *);
    void LoadOnlySongData(SongInfo *, bool, HamSongDataValidate);
    void ResetAudio();
    float StreamMs() const;
    bool DetectStreamJump(float &, float &, float &) const;
    float EventBeat(Symbol);
    void AddMusicFader(Fader *);
    void SetMaps();
    void LoaderPoll();
    int Unk70() const { return mSongPos.GetMeasure(); }
    float TotalBeat1() const { return mSongPos.GetTotalBeat(); }
    float TotalBeat2() const { return mLastSongPos.GetTotalBeat(); }
    HamAudio *GetAudio() const { return mAudio; }
    HamSongData *SongData() const { return mSongData; }
    MidiParserMgr *GetMidiParserMgr() const { return mMidiParserMgr; }
    const SongPos &Pos1() const { return mSongPos; }
    const SongPos &Pos2() const { return mLastSongPos; }

private:
    void CheckBeat();
    void CheckLevels();

    HamSongData *mSongData; // 0x30
    HamAudio *mAudio; // 0x34
    MidiParserMgr *mMidiParserMgr; // 0x38
    SongInfo *mSongInfo; // 0x3c
    HamMasterLoader *mLoader; // 0x40
    bool mSongStream;
    bool mLoaded;
    float mLastStreamMs;
    float mStreamMs; // 0x4c
    bool mLoopBack;
    float mLoopBackFromStreamMs;
    float mLastLoopFromMs;
    float mLastLoopToMs;
    SongPos mSongPos;
    SongPos mLastSongPos;
    std::vector<int> mSubmixIdxs;
    std::pair<float, float> mCurLevel;
    // float unk9c;
    // float unka0;
    float mLevelHighWaterMark;
    std::list<Vector2> mLevelHistorisis;
    int mActivePlayerTrack;
    int mCurActivePlayers;
    int mActivePlayerGemIdx;
    bool mMetronome; // 0xbc
};

extern HamMaster *TheMaster;
