#include "synth/FxSendEQ.h"
#include "FxSendEQ.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/BinStream.h"

FxSendEQ::FxSendEQ()
    : mHighFreqCutoff(12000.0f), mHighFreqGain(0.0f), mMidFreqCutoff(8000.0f),
      mMidFreqBandwidth(1000.0f), mMidFreqGain(0.0f), mLowFreqCutoff(2000.0f),
      mLowFreqGain(0.0f), mLPFCutoff(20000.0f), mLPFReso(0.0f), mHPFCutoff(20.0f),
      mHPFReso(0.0f), mLRMode(false), mTransitionTime(25.0f) {}

FxSendEQ::~FxSendEQ() {}

BEGIN_COPYS(FxSendEQ)
    COPY_SUPERCLASS(FxSend)
    CREATE_COPY(FxSendEQ)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mHighFreqCutoff)
        COPY_MEMBER(mHighFreqGain)
        COPY_MEMBER(mMidFreqCutoff)
        COPY_MEMBER(mMidFreqBandwidth)
        COPY_MEMBER(mMidFreqGain)
        COPY_MEMBER(mLowFreqCutoff)
        COPY_MEMBER(mLowFreqGain)
        COPY_MEMBER(mLPFCutoff)
        COPY_MEMBER(mLPFReso)
        COPY_MEMBER(mHPFCutoff)
        COPY_MEMBER(mHPFReso)
        COPY_MEMBER(mLRMode)
        COPY_MEMBER(mTransitionTime)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_SAVES(FxSendEQ)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(FxSend)
    bs << mHighFreqCutoff;
    bs << mHighFreqGain;
    bs << mMidFreqCutoff;
    bs << mMidFreqBandwidth;
    bs << mMidFreqGain;
    bs << mLowFreqCutoff;
    bs << mLowFreqGain;
    bs << mLPFCutoff;
    bs << mLPFReso;
    bs << mHPFCutoff;
    bs << mHPFReso;
    bs << mLRMode;
    bs << mTransitionTime;
END_SAVES

INIT_REVS(3, 0)

BEGIN_LOADS(FxSendEQ)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(FxSend)
    d >> mHighFreqCutoff >> mHighFreqGain >> mMidFreqCutoff >> mMidFreqBandwidth
        >> mMidFreqGain >> mLowFreqCutoff >> mLowFreqGain;
    if (d.rev >= 2) {
        d >> mLPFCutoff >> mLPFReso >> mHPFCutoff >> mHPFReso;
    }
    if (d.rev >= 3) {
        d >> mLRMode;
        d >> mTransitionTime;
    }
    OnParametersChanged();
END_LOADS

BEGIN_HANDLERS(FxSendEQ)
    HANDLE_SUPERCLASS(FxSend)
END_HANDLERS

BEGIN_PROPSYNCS(FxSendEQ)
    SYNC_PROP_MODIFY(high_frequency, mHighFreqCutoff, OnParametersChanged())
    SYNC_PROP_MODIFY(high_gain, mHighFreqGain, OnParametersChanged())
    SYNC_PROP_MODIFY(mid_frequency, mMidFreqCutoff, OnParametersChanged())
    SYNC_PROP_MODIFY(mid_bandwidth, mMidFreqBandwidth, OnParametersChanged())
    SYNC_PROP_MODIFY(mid_gain, mMidFreqGain, OnParametersChanged())
    SYNC_PROP_MODIFY(low_frequency, mLowFreqCutoff, OnParametersChanged())
    SYNC_PROP_MODIFY(low_gain, mLowFreqGain, OnParametersChanged())
    SYNC_PROP_MODIFY(low_pass_cutoff, mLPFCutoff, OnParametersChanged())
    SYNC_PROP_MODIFY(low_pass_reso, mLPFReso, OnParametersChanged())
    SYNC_PROP_MODIFY(high_pass_cutoff, mHPFCutoff, OnParametersChanged())
    SYNC_PROP_MODIFY(high_pass_reso, mHPFReso, OnParametersChanged())
    SYNC_PROP_MODIFY(lr_mode, mLRMode, OnParametersChanged())
    SYNC_PROP_MODIFY(transition_time, mTransitionTime, OnParametersChanged())
    SYNC_SUPERCLASS(FxSend)
END_PROPSYNCS
