#include "synth/FxSendFlanger.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/BinStream.h"

BEGIN_COPYS(FxSendFlanger)
    COPY_SUPERCLASS(FxSend)
    CREATE_COPY(FxSendFlanger)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDelayMs)
        COPY_MEMBER(mRate)
        COPY_MEMBER(mDepthPct)
        COPY_MEMBER(mFeedbackPct)
        COPY_MEMBER(mOffsetPct)
        COPY_MEMBER(mTempoSync)
        COPY_MEMBER(mSyncType)
        COPY_MEMBER(mTempo)
    END_COPYING_MEMBERS
END_COPYS

FxSendFlanger::FxSendFlanger()
    : mDelayMs(2.0f), mRate(0.5f), mDepthPct(50), mFeedbackPct(50), mOffsetPct(10),
      mTempoSync(0), mSyncType(), mTempo(120.0f) {
    static Symbol quarter("quarter");
    mSyncType = quarter;
    mDryGain = -3.0f;
    mWetGain = -3.0f;
}

void FxSendFlanger::Save(BinStream &bs) {
    bs << 7;
    SAVE_SUPERCLASS(FxSend)
    bs << mDelayMs;
    bs << mRate;
    bs << mDepthPct;
    bs << mFeedbackPct;
    bs << mOffsetPct;
    bs << mTempoSync;
    bs << mSyncType;
    bs << mTempo;
}

BEGIN_LOADS(FxSendFlanger)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    LOAD_SUPERCLASS(FxSend)
    if (d.rev <= 4) {
        mDryGain = -3.0f;
        mWetGain = -3.0f;
        UpdateMix();
    }
    bs >> mDelayMs >> mRate;
    int dummy;

    if (d.rev >= 4)
        bs >> mDepthPct;
    else
        bs >> dummy;
    if (d.rev >= 2)
        bs >> mFeedbackPct;
    if (d.rev >= 3)
        bs >> mOffsetPct;

    if (d.rev >= 6) {
        d >> mTempoSync;
        bs >> mSyncType;
        bs >> mTempo;
    }
    OnParametersChanged();
END_LOADS

BEGIN_HANDLERS(FxSendFlanger)
    HANDLE_SUPERCLASS(FxSend)
END_HANDLERS

BEGIN_PROPSYNCS(FxSendFlanger)
    SYNC_PROP_MODIFY(delay_ms, mDelayMs, OnParametersChanged())
    SYNC_PROP_MODIFY(rate, mRate, OnParametersChanged())
    SYNC_PROP_MODIFY(depth_pct, mDepthPct, OnParametersChanged())
    SYNC_PROP_MODIFY(feedback_pct, mFeedbackPct, OnParametersChanged())
    SYNC_PROP_MODIFY(offset_pct, mOffsetPct, OnParametersChanged())
    SYNC_PROP_MODIFY(tempo_sync, mTempoSync, OnParametersChanged())
    SYNC_PROP_MODIFY(sync_type, mSyncType, OnParametersChanged())
    SYNC_PROP_MODIFY(tempo, mTempo, OnParametersChanged())
    SYNC_SUPERCLASS(FxSend)
END_PROPSYNCS
