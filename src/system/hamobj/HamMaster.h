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
    virtual bool IsLoaded() { return unk45; }

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
    int Unk70() const { return unk60.GetMeasure(); }
    HamAudio *GetAudio() const { return mAudio; }
    HamSongData *SongData() const { return mSongData; }
    MidiParserMgr *GetMidiParserMgr() const { return mMidiParserMgr; }

private:
    HamSongData *mSongData; // 0x30
    HamAudio *mAudio; // 0x34
    MidiParserMgr *mMidiParserMgr; // 0x38
    SongInfo *mSongInfo; // 0x3c
    HamMasterLoader *mLoader; // 0x40
    bool unk44;
    bool unk45;
    float unk48;
    float mStreamMs; // 0x4c
    bool unk50;
    float unk54;
    float unk58;
    float unk5c;
    SongPos unk60;
    SongPos unk78;
    std::vector<int> unk90;
    float unk9c;
    float unka0;
    float unka4;
    std::list<Vector2> unka8;
    int unkb0;
    int unkb4;
    int unkb8;
    bool mMetronome; // 0xbc
};

extern HamMaster *TheMaster;
