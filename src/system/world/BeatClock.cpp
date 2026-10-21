#include "world/BeatClock.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Poll.h"
#include "utl/MeasureMap.h"
#include "utl/SongPos.h"

BeatClock::BeatClock()
    : mMeasureMap(new MeasureMap()), mSound(this), mBpm(100), mBeatsPerMeasure(4),
      mMeasuresPerPhrase(0), mUseGlobal(0), mElapsedTime(0), mLastTime(0), mIsRunning(0),
      mTimeUnits(kTaskSeconds) {}

BeatClock::~BeatClock() { RELEASE(mMeasureMap); }

BEGIN_HANDLERS(BeatClock)
    HANDLE_ACTION(start, Start())
    HANDLE_ACTION(pause, Pause())
    HANDLE_ACTION(reset, Reset())
    HANDLE(sync, OnSyncState)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(BeatClock)
    SYNC_PROP(bpm, mBpm)
    SYNC_PROP_SET(beats_per_measure, mBeatsPerMeasure, SetBeatsPerMeasure(_val.Int()))
    SYNC_PROP(measures_per_phrase, mMeasuresPerPhrase)
    SYNC_PROP_MODIFY(use_global, mUseGlobal, mSound = nullptr)
    SYNC_PROP(measure, mSongPos.mMeasure)
    SYNC_PROP(phrase, mSongPos.mPhrase)
    SYNC_PROP(beat, mSongPos.mBeat)
    SYNC_PROP(tick, mSongPos.mTick)
    SYNC_PROP(sub_division, mSubDivision)
    SYNC_PROP(total_beat, mSongPos.mTotalBeat)
    SYNC_PROP(seconds, mElapsedTime)
    SYNC_PROP_MODIFY(sound, mSound, mUseGlobal = false)
    SYNC_PROP(timeline, (int &)mTimeUnits)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(BeatClock)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mBpm;
    bs << mBeatsPerMeasure;
    bs << mUseGlobal;
    bs << mMeasuresPerPhrase;
    bs << mSound;
    bs << mTimeUnits;
END_SAVES

BEGIN_COPYS(BeatClock)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(BeatClock)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mBpm)
        COPY_MEMBER(mBeatsPerMeasure)
        COPY_MEMBER(mMeasuresPerPhrase)
        COPY_MEMBER(mUseGlobal)
        COPY_MEMBER(mElapsedTime)
        COPY_MEMBER(mSound)
        COPY_MEMBER(mTimeUnits)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(BeatClock)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mBpm;
    d >> mBeatsPerMeasure;
    d >> mUseGlobal;
    if (d.rev >= 1) {
        d >> mMeasuresPerPhrase;
    }
    if (d.rev >= 2) {
        d >> mSound;
    }
    if (d.rev >= 3) {
        d >> (int &)mTimeUnits;
    }
    SetBeatsPerMeasure(mBeatsPerMeasure);
END_LOADS

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

void BeatClock::UpdateSongPos() {
    SongPos pos;
    float secs = 0;
    if (mUseGlobal) {
        pos = TheTaskMgr.GetSongPos();
        secs = TheTaskMgr.Time(mTimeUnits);
    } else {
        if (mSound) {
            secs = mSound->ElapsedTime();
        } else {
            float time = TheTaskMgr.Time(mTimeUnits);
            float old50 = mLastTime;
            mLastTime = time;
            if (time - old50 == 0) {
                return;
            }
            if (mIsRunning == 0) {
                return;
            }
            secs = mElapsedTime + (time - old50);
        }
        pos.mTotalBeat = mBpm * secs / 60.0f;
        pos.mTotalTick = pos.GetTotalBeat() * 480.0f;
        mMeasureMap->TickToMeasureBeatTick(
            pos.mTotalTick, pos.mMeasure, pos.mBeat, pos.mTick
        );
    }
    if (mMeasuresPerPhrase != 0) {
        pos.mPhrase = pos.GetMeasure() / mMeasuresPerPhrase;
        pos.mMeasure = pos.GetMeasure() % mMeasuresPerPhrase;
    }
    int div = pos.GetTick() / 120;
    if (mMeasuresPerPhrase != 0) {
        SetProperty("phrase", pos.GetPhrase());
    }
    SetProperty("measure", pos.GetMeasure());
    SetProperty("beat", pos.GetBeat());
    SetProperty("tick", pos.GetTick());
    SetProperty("total_beat", pos.GetTotalBeat());
    SetProperty("sub_division", div);
    SetProperty("seconds", secs);
}

DataNode BeatClock::OnSyncState(DataArray *a) {
    BeatClock *other = a->Obj<BeatClock>(2);
    int i3 = a->Int(3);
    if (other) {
        SongPos aSongPos = other->mSongPos;
        SongPos mySongPos = mSongPos;
        switch (i3) {
        case 0:
            SetProperty("tick", aSongPos.GetTick());
            SetProperty("sub_division", other->mSubDivision);
            break;
        case 1:
            SetProperty("tick", aSongPos.GetTick());
            SetProperty("sub_division", other->mSubDivision);
            SetProperty("beat", aSongPos.GetBeat());
            break;
        case 2:
            SetProperty("tick", aSongPos.GetTick());
            SetProperty("sub_division", other->mSubDivision);
            SetProperty("beat", aSongPos.GetBeat());
            SetProperty("measure", aSongPos.GetMeasure());
            break;
        default:
            break;
        }
        float mdiff = (mSongPos.GetMeasure() - mySongPos.GetMeasure()) * mBeatsPerMeasure;
        float bdiff = mSongPos.GetBeat() - mySongPos.GetBeat();
        float tdiff = (float)(mSongPos.GetTick() - mySongPos.GetTick()) / 480.0f;
        float f5 = bdiff + tdiff;
        f5 += mdiff;
        if (!NearlyZero(f5)) {
            mSongPos.mTotalBeat += f5;
            mSongPos.mTotalTick = mSongPos.mTotalBeat * 480.0f;
            BroadcastPropertyChange("total_beat");
            mElapsedTime += (60.0f / mBpm) * f5;
            BroadcastPropertyChange("seconds");
        }
    }
    return 0;
}
