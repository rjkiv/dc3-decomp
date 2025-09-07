#include "flow/FlowDistance.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"

FlowDistance::FlowDistance()
    : mObj1(this, nullptr), mObj2(this, nullptr), mDistance(10), mPersistent(0), unka1(0),
      unka2(0), mRunInRange(1), mDriveIntensity(0), unka8(0) {}

FlowDistance::~FlowDistance() {}

BEGIN_HANDLERS(FlowDistance)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowDistance)
    SYNC_PROP(one, mObj1)
    SYNC_PROP(two, mObj2)
    SYNC_PROP(distance, mDistance)
    SYNC_PROP(persistent, mPersistent)
    SYNC_PROP(run_in_range, mRunInRange)
    SYNC_PROP(drive_intensity, mDriveIntensity)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowDistance)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mObj1;
    bs << mObj2;
    bs << mPersistent;
    bs << mDistance;
    bs << mRunInRange;
    bs << mDriveIntensity;
END_SAVES

BEGIN_COPYS(FlowDistance)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowDistance)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mObj1)
        COPY_MEMBER(mObj2)
        COPY_MEMBER(mPersistent)
        COPY_MEMBER(mDistance)
        COPY_MEMBER(mRunInRange)
        COPY_MEMBER(mDriveIntensity)
    END_COPYING_MEMBERS
END_COPYS
