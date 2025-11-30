#include "synth/FxSendDistortion.h"
#include "obj/Object.h"
#include "synth/FxSend.h"
#include "utl/BinStream.h"

FxSendDistortion::FxSendDistortion() : mDrive(0) {}

FxSendDistortion::~FxSendDistortion() {}

BEGIN_COPYS(FxSendDistortion)
    COPY_SUPERCLASS(FxSend)
    CREATE_COPY(FxSendDistortion)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDrive)
    END_COPYING_MEMBERS
END_COPYS

void FxSendDistortion::Save(BinStream &bs) {
    bs << 1;
    SAVE_SUPERCLASS(FxSend)
    bs << mDrive;
}

BEGIN_LOADS(FxSendDistortion)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FxSend)
    bs >> mDrive;
    OnParametersChanged();
END_LOADS

BEGIN_HANDLERS(FxSendDistortion)
    HANDLE_SUPERCLASS(FxSend)
END_HANDLERS

BEGIN_PROPSYNCS(FxSendDistortion)
    SYNC_PROP_MODIFY(drive, mDrive, OnParametersChanged())
    SYNC_SUPERCLASS(FxSend)
END_PROPSYNCS
