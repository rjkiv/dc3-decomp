#pragma once
#include "beatmatch/GemListInterface.h"
#include "beatmatch/HxMaster.h"
#include "beatmatch/HxSongData.h"
#include "midi/Midi.h"
#include "utl/BeatMap.h"
#include "utl/MeasureMap.h"
#include "utl/MemStream.h"
#include "utl/SongInfoCopy.h"
#include "utl/TempoMap.h"

enum HamSongDataValidate {
    // SongDataValidate values from RB3
    // kSongData_NoValidation,
    // kSongData_ValidateUsingNameOnly,
    // kSongData_Validate
};

class HamSongData : public GemListInterface, public HxSongData, public MidiReceiver {
public:
    HamSongData();
    // GemListInterface
    virtual ~HamSongData();
    virtual void SetTrack(Symbol) {}
    virtual bool GetGem(int, int &, int &, int &) { return false; }
    // HxSongData
    virtual SongPos CalcSongPos(HxMaster *, float);
    virtual TempoMap *GetTempoMap() const { return mTempoMap; }
    virtual BeatMap *GetBeatMap() const { return mBeatMap; }
    virtual MeasureMap *GetMeasureMap() const { return mMeasureMap; }
    // MidiReceiver
    virtual void OnNewTrack(int) {}
    virtual void OnEndOfTrack() {}
    virtual void OnAllTracksRead() {}
    virtual void OnMidiMessage(
        int tick, unsigned char status, unsigned char data1, unsigned char data2
    ) {}
    virtual void OnText(int tick, const char *text, unsigned char type) {}
    virtual void OnTempo(int, int) {}
    virtual void OnTimeSig(int, int, int) {}

    static HamSongData *sInstance;

    void Load(const SongInfo *, bool, HamSongDataValidate);
    void SetMaps();
    bool Poll();

private:
    void Load(const char *, const SongInfo *, bool);
    void PostLoad();

    bool unk10;
    const SongInfo *unk14;
    TempoMap *mTempoMap; // 0x18
    MeasureMap *mMeasureMap; // 0x1c
    BeatMap *mBeatMap; // 0x20
    MemStream *mStream; // 0x24
    MidiReader *mMidiReader; // 0x28
};
