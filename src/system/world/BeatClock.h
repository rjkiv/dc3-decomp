#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Poll.h"
#include "synth/Sound.h"
#include "utl/MeasureMap.h"
#include "utl/MemMgr.h"
#include "utl/SongPos.h"

/** "Class that provides a beat using the taskmgr time or uses the taskmgr beat" */
class BeatClock : public RndPollable {
public:
    enum SyncMode {
        /** "Copy only the fractional part of the current beat over" */
        kSync_TicksOnly = 0,
        /** "Copy over beat and ticks" */
        kSync_BeatsAndTicks = 1,
        /** "Copy measure, beat, and ticks" */
        kSync_MeasuresBeatsAndTicks = 2
    };
    // Hmx::Object
    virtual ~BeatClock();
    OBJ_CLASSNAME(BeatClock);
    OBJ_SET_TYPE(BeatClock);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    OBJ_MEM_OVERLOAD(0x1B)
    NEW_OBJ(BeatClock)

    void SetBeatsPerMeasure(int);
    void Reset();

protected:
    BeatClock();

    void UpdateSongPos();
    DataNode OnSyncState(DataArray *);

    MeasureMap *mMeasureMap; // 0x8
    /** "Sound object to which this BeatClock's time is synchronized." */
    ObjPtr<Sound> mSound; // 0xc
    /** "beats per minute" */
    float mBeatsPerMinute; // 0x20
    /** "beats per measure" */
    int mBeatsPerMeasure; // 0x24
    /** "Measures per phrase.  Use 0 to disable phrases." */
    int mMeasuresPerPhrase; // 0x28
    /** "uses the global task mgr time if true" */
    bool mUseGlobal; // 0x2c
    /** The current song position. (MBT, phrase, etc) */
    SongPos mSongPos; // 0x30
    /** "Current 16th note subdivision of the beat" */
    int mSubDivision; // 0x48
    /** "Total seconds" */
    float mTotalSeconds; // 0x4c
    float unk50; // 0x50
    bool unk54; // 0x54
    /** "which timeline does this beatclock run on?" */
    TaskUnits mTimeline; // 0x58
};
