#include "synth/FxSendMeterEffect.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/BinStream.h"

BEGIN_COPYS(FxSendMeterEffect)
    COPY_SUPERCLASS(FxSend)
    CREATE_COPY(FxSendMeterEffect)
END_COPYS

void FxSendMeterEffect::Save(BinStream &bs) {
    bs << 1;
    SAVE_SUPERCLASS(FxSend)
}

BEGIN_LOADS(FxSendMeterEffect)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FxSend)
    OnParametersChanged();
END_LOADS

float FxSendMeterEffect::ChannelData(int idx) {
    if (mChannels.size() == 0)
        return 0.0f;
    int lastIdx = mChannels.size() - 1;
    if (lastIdx < idx)
        idx = lastIdx;
    return mChannels[idx].mPeak;
}

BEGIN_HANDLERS(FxSendMeterEffect)
    HANDLE_EXPR(channel1, ChannelData(0))
    HANDLE_EXPR(channel2, ChannelData(1))
    HANDLE_SUPERCLASS(FxSend)
END_HANDLERS

BEGIN_PROPSYNCS(FxSendMeterEffect)
    SYNC_PROP_MODIFY(reset_peaks, mResetPeaks, OnParametersChanged())
    SYNC_PROP_SET(channel1, ChannelData(0), )
    SYNC_PROP_SET(channel2, ChannelData(1), )
    SYNC_SUPERCLASS(FxSend)
END_PROPSYNCS

FxSendMeterEffect::FxSendMeterEffect() : mResetPeaks(0) {}
