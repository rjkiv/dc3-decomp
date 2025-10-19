#include "synth/FxSendChorus.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/BinStream.h"

BEGIN_COPYS(FxSendChorus)
    COPY_SUPERCLASS(FxSend)
    CREATE_COPY(FxSendChorus)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDelayMs)
        COPY_MEMBER(mRate)
        COPY_MEMBER(mDepth)
        COPY_MEMBER(mFeedbackPct)
        COPY_MEMBER(mOffsetPct)
        COPY_MEMBER(mTempoSync)
        COPY_MEMBER(mSyncType)
        COPY_MEMBER(mTempo)
    END_COPYING_MEMBERS
END_COPYS

FxSendChorus::FxSendChorus()
    : mDelayMs(50.0f), mRate(1.0f), mDepth(10.0f), mFeedbackPct(30), mOffsetPct(20),
      mTempoSync(0), mSyncType(), mTempo(120.0f) {
    static Symbol quarter("quarter");
    mSyncType = quarter;
    mDryGain = -3.0f;
    mWetGain = -3.0f;
}

void FxSendChorus::Save(BinStream &bs) {
    bs << 4;
    SAVE_SUPERCLASS(FxSend)
    bs << mDelayMs;
    bs << mRate;
    bs << mDepth;
    bs << mFeedbackPct;
    bs << mOffsetPct;
    bs << mTempoSync;
    bs << mSyncType;
    bs << mTempo;
}

BEGIN_LOADS(FxSendChorus)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FxSend)
    if (d.rev == 1) {
        mDryGain = -3.0f;
        mWetGain = -3.0f;
        UpdateMix();
    }
    bs >> mDelayMs >> mRate >> mDepth;
    if (d.rev < 4) {
        int feedbackInt;
        bs >> feedbackInt;
        mFeedbackPct = feedbackInt;
        int offsetInt;
        bs >> offsetInt;
        mOffsetPct = offsetInt;
    } else {
        bs >> mFeedbackPct >> mOffsetPct;
    }
    if (d.rev >= 3) {
        d >> mTempoSync;
        bs >> mSyncType;
        bs >> mTempo;
    }
    OnParametersChanged();
END_LOADS

BEGIN_HANDLERS(FxSendChorus)
    HANDLE_SUPERCLASS(FxSend)
END_HANDLERS

BEGIN_PROPSYNCS(FxSendChorus)
    SYNC_PROP_MODIFY(delay_ms, mDelayMs, OnParametersChanged())
    SYNC_PROP_MODIFY(rate, mRate, OnParametersChanged())
    SYNC_PROP_MODIFY(depth, mDepth, OnParametersChanged())
    SYNC_PROP_MODIFY(feedback_pct, mFeedbackPct, OnParametersChanged())
    SYNC_PROP_MODIFY(offset_pct, mOffsetPct, OnParametersChanged())
    SYNC_PROP_MODIFY(tempo_sync, mTempoSync, OnParametersChanged())
    SYNC_PROP_MODIFY(sync_type, mSyncType, OnParametersChanged())
    SYNC_PROP_MODIFY(tempo, mTempo, OnParametersChanged())
    SYNC_SUPERCLASS(FxSend)
END_PROPSYNCS
