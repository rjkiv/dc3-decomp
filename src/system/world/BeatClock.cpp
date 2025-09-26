#include "world/BeatClock.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"
#include "utl/MeasureMap.h"

BeatClock::BeatClock()
    : mMeasureMap(new MeasureMap()), mSound(this), mBeatsPerMinute(100),
      mBeatsPerMeasure(4), mMeasuresPerPhrase(0), mUseGlobal(0), mTotalSeconds(0),
      unk50(0), unk54(0), mTimeline(kTaskSeconds) {}

BeatClock::~BeatClock() { RELEASE(mMeasureMap); }

BEGIN_HANDLERS(BeatClock)
    HANDLE_ACTION(start, unk54 = true)
    HANDLE_ACTION(pause, unk54 = false)
    HANDLE_ACTION(reset, Reset())
    HANDLE(sync, OnSyncState)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(BeatClock)
    SYNC_PROP(bpm, mBeatsPerMinute)
    SYNC_PROP_SET(beats_per_measure, mBeatsPerMeasure, SetBeatsPerMeasure(_val.Int()))
    SYNC_PROP(measures_per_phrase, mMeasuresPerPhrase)
    SYNC_PROP_MODIFY(use_global, mUseGlobal, mSound = nullptr)
    SYNC_PROP(measure, mSongPos.AccessMeasure())
    SYNC_PROP(phrase, mSongPos.AccessPhrase())
    SYNC_PROP(beat, mSongPos.AccessBeat())
    SYNC_PROP(tick, mSongPos.AccessTick())
    SYNC_PROP(sub_division, mSubDivision)
    SYNC_PROP(total_beat, mSongPos.AccessTotalBeat())
    SYNC_PROP(seconds, mTotalSeconds)
    SYNC_PROP_MODIFY(sound, mSound, mUseGlobal = false)
    SYNC_PROP(timeline, (int &)mTimeline)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(BeatClock)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mBeatsPerMinute;
    bs << mBeatsPerMeasure;
    bs << mUseGlobal;
    bs << mMeasuresPerPhrase;
    bs << mSound;
    bs << mTimeline;
END_SAVES

BEGIN_COPYS(BeatClock)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(BeatClock)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBeatsPerMinute)
        COPY_MEMBER(mBeatsPerMeasure)
        COPY_MEMBER(mMeasuresPerPhrase)
        COPY_MEMBER(mUseGlobal)
        COPY_MEMBER(mTotalSeconds)
        COPY_MEMBER(mSound)
        COPY_MEMBER(mTimeline)
    END_COPYING_MEMBERS
END_COPYS

void BeatClock::Enter() { Reset(); }
void BeatClock::Poll() { UpdateSongPos(); }

void BeatClock::Reset() {
    if (!mUseGlobal && !mSound) {
        if (mMeasuresPerPhrase != 0) {
            SetProperty("phrase", 0);
        }
        SetProperty("measure", 0);
        SetProperty("beat", 0);
        SetProperty("total_beat", 0.0f);
        SetProperty("sub_division", 0);
        SetProperty("seconds", 0.0f);
        static Message on_reset("on_reset");
        Export(on_reset, true);
    }
}

void BeatClock::SetBeatsPerMeasure(int beats) {
    mBeatsPerMeasure = beats;
    RELEASE(mMeasureMap);
    mMeasureMap = new MeasureMap();
    mMeasureMap->AddTimeSignature(0, beats, 4, true);
}
