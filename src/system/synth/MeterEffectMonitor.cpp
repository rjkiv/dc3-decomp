#include "synth/MeterEffectMonitor.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "rndobj/Poll.h"

MeterEffectMonitor::MeterEffectMonitor()
    : mMeterEffect(this), mLastChannel1(0), mLastChannel2(0) {}
MeterEffectMonitor::~MeterEffectMonitor() {}

BEGIN_HANDLERS(MeterEffectMonitor)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(MeterEffectMonitor)
    SYNC_PROP(meter_effect, mMeterEffect)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(MeterEffectMonitor)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMeterEffect;
END_SAVES

BEGIN_COPYS(MeterEffectMonitor)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(MeterEffectMonitor)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMeterEffect)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(MeterEffectMonitor)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mMeterEffect;
END_LOADS

void MeterEffectMonitor::Poll() {
    float data1 = 0;
    float data2 = 0;
    if (mMeterEffect) {
        data1 = mMeterEffect->ChannelData(0);
        data2 = mMeterEffect->ChannelData(1);
    }
    if (data1 != mLastChannel1 || data2 != mLastChannel2) {
        static Message channel_data("channel_data", 0.0f, 0.0f);
        channel_data[0] = (mLastChannel1 + data1) / 2.0f;
        channel_data[1] = (mLastChannel2 + data2) / 2.0f;
        Export(channel_data, true);
        mLastChannel1 = data1;
        mLastChannel2 = data2;
    }
}
