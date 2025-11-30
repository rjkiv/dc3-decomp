#include "synth/FxSendReverb.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/BinStream.h"

BEGIN_COPYS(FxSendReverb)
    COPY_SUPERCLASS(FxSend)
    CREATE_COPY(FxSendReverb)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mEnvironmentPreset)
        COPY_MEMBER(mPreDelayMs)
        COPY_MEMBER(mHighCut)
        COPY_MEMBER(mLowCut)
        COPY_MEMBER(mRoomSize)
        COPY_MEMBER(mDamping)
        COPY_MEMBER(mDiffusion)
        COPY_MEMBER(mEarlyLate)
    END_COPYING_MEMBERS
END_COPYS

FxSendReverb::FxSendReverb()
    : mEnvironmentPreset(), mPreDelayMs(50.0f), mHighCut(5000.0f), mLowCut(100.0f),
      mRoomSize(0.5f), mDamping(0.5f), mDiffusion(0.5f), mEarlyLate(0.5f) {
    static Symbol generic("generic");
    mEnvironmentPreset = generic;
    mDryGain = 0.0f;
    mWetGain = -6.0f;
}

FxSendReverb::~FxSendReverb() {}

void FxSendReverb::Save(BinStream &bs) {
    bs << 2;
    SAVE_SUPERCLASS(FxSend)
    bs << mEnvironmentPreset;
    bs << mPreDelayMs << mHighCut << mLowCut << mRoomSize << mDamping << mDiffusion
       << mEarlyLate;
}

BEGIN_LOADS(FxSendReverb)
    LOAD_REVS(bs);
    ASSERT_REVS(2, 0);
    LOAD_SUPERCLASS(FxSend)
    bs >> mEnvironmentPreset;
    if (d.rev >= 2) {
        bs >> mPreDelayMs >> mHighCut >> mLowCut >> mRoomSize >> mDamping >> mDiffusion
            >> mEarlyLate;
    }
    OnParametersChanged();
END_LOADS

BEGIN_HANDLERS(FxSendReverb)
    HANDLE_SUPERCLASS(FxSend)
END_HANDLERS

BEGIN_PROPSYNCS(FxSendReverb)
    SYNC_PROP_MODIFY(environment, mEnvironmentPreset, OnParametersChanged())
    SYNC_PROP_MODIFY(pre_delay_ms, mPreDelayMs, OnParametersChanged())
    SYNC_PROP_MODIFY(high_cut, mHighCut, OnParametersChanged())
    SYNC_PROP_MODIFY(low_cut, mLowCut, OnParametersChanged())
    SYNC_PROP_MODIFY(room_size, mRoomSize, OnParametersChanged())
    SYNC_PROP_MODIFY(damping, mDamping, OnParametersChanged())
    SYNC_PROP_MODIFY(diffusion, mDiffusion, OnParametersChanged())
    SYNC_PROP_MODIFY(early_late, mEarlyLate, OnParametersChanged())
    SYNC_SUPERCLASS(FxSend)
END_PROPSYNCS
