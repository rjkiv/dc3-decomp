#include "hamobj/HamSupereasyData.h"
#include "obj/Object.h"

HamSupereasyData::HamSupereasyData() {}
HamSupereasyData::~HamSupereasyData() {}

BEGIN_HANDLERS(HamSupereasyData)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(HamSupereasyMeasure)
    SYNC_PROP(first, o.first)
    SYNC_PROP(second, o.second)
    SYNC_PROP(preferred, o.preferred)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(HamSupereasyData)
    SYNC_PROP(routine, mRoutine)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamSupereasyData)
    SAVE_REVS(0, 1)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mRoutine.size();
    for (std::vector<HamSupereasyMeasure>::iterator it = mRoutine.begin();
         it != mRoutine.end();
         ++it) {
        bs << it->first;
        bs << it->second;
        bs << it->preferred;
    }
END_SAVES

BEGIN_COPYS(HamSupereasyData)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(HamSupereasyData)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mRoutine)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(HamSupereasyData)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 1)
    LOAD_SUPERCLASS(Hmx::Object)
    int numMeasures;
    bs >> numMeasures;
    mRoutine.resize(numMeasures);
    for (std::vector<HamSupereasyMeasure>::iterator it = mRoutine.begin();
         it != mRoutine.end();
         ++it) {
        bs >> it->first;
        bs >> it->second;
        if (bsrev.altRev > 0)
            bs >> it->preferred;
    }
END_LOADS
