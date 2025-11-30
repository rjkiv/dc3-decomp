#include "synth/FxSendDelay.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/BinStream.h"

BEGIN_COPYS(FxSendDelay)
    COPY_SUPERCLASS(FxSend)
    CREATE_COPY(FxSendDelay)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDelayTime)
        COPY_MEMBER(mGain)
        COPY_MEMBER(mTempoSync)
        COPY_MEMBER(mSyncType)
        COPY_MEMBER(mTempo)
        COPY_MEMBER(mPingPongPct)
    END_COPYING_MEMBERS
END_COPYS

FxSendDelay::FxSendDelay()
    : mDelayTime(0.2f), mGain(-6.0f), mPingPongPct(0.0f), mTempoSync(0), mSyncType(),
      mTempo(120.0f) {
    static Symbol eighth("eighth");
    mSyncType = eighth;
}

FxSendDelay::~FxSendDelay() {}

void FxSendDelay::Save(BinStream &bs) {
    bs << 3;
    SAVE_SUPERCLASS(FxSend)
    bs << mDelayTime;
    bs << mGain;
    bs << mTempoSync;
    bs << mSyncType;
    bs << mTempo;
    bs << mPingPongPct;
}

BEGIN_LOADS(FxSendDelay)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FxSend)
    bs >> mDelayTime >> mGain;
    if (d.rev >= 2) {
        d >> mTempoSync;
        bs >> mSyncType;
        bs >> mTempo;
    }
    if (d.rev >= 3) {
        bs >> mPingPongPct;
    }
    OnParametersChanged();
END_LOADS

BEGIN_HANDLERS(FxSendDelay)
    HANDLE_SUPERCLASS(FxSend)
END_HANDLERS

BEGIN_PROPSYNCS(FxSendDelay)
    SYNC_PROP_MODIFY(delay_time, mDelayTime, OnParametersChanged())
    SYNC_PROP_MODIFY(feedback, mGain, OnParametersChanged())
    SYNC_PROP_MODIFY(tempo_sync, mTempoSync, OnParametersChanged())
    SYNC_PROP_MODIFY(sync_type, mSyncType, OnParametersChanged())
    SYNC_PROP_MODIFY(tempo, mTempo, OnParametersChanged())
    SYNC_PROP_MODIFY(ping_pong_pct, mPingPongPct, OnParametersChanged())
    SYNC_SUPERCLASS(FxSend)
END_PROPSYNCS
