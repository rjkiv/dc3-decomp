#include "synth/FxSendBitCrush.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/BinStream.h"

FxSendBitCrush::FxSendBitCrush() : mAmount(0) {}

FxSendBitCrush::~FxSendBitCrush() {}

BEGIN_COPYS(FxSendBitCrush)
    COPY_SUPERCLASS(FxSend)
    CREATE_COPY(FxSendBitCrush)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mAmount)
    END_COPYING_MEMBERS
END_COPYS

void FxSendBitCrush::Save(BinStream &bs) {
    bs << 1;
    SAVE_SUPERCLASS(FxSend)
    bs << mAmount;
}

BEGIN_LOADS(FxSendBitCrush)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FxSend)
    bs >> mAmount;
    OnParametersChanged();
END_LOADS

BEGIN_PROPSYNCS(FxSendBitCrush)
    SYNC_PROP_MODIFY(amount, mAmount, OnParametersChanged())
    SYNC_SUPERCLASS(FxSend)
END_PROPSYNCS
