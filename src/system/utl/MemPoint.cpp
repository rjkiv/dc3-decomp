#include "MemPoint.h"
#include "os/Memory.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/MemMgr.h"
#include "string.h"
#include "Str.h"

MemPoint::MemPoint(eInitType t) {
    if (t == kSnapshot) {
        for (int i = 0; i < MemNumHeaps(); i++) {
            int a, b, c, d;
            MemFreeBlockStats(i, a, b, mHeapFree[i], c, d);
        }
        mPhysUsed = PhysicalUsage();
    } else {
        memset(this, 0, sizeof(MemPoint));
    }
}

MemPointDelta MemPoint::operator-(const MemPoint &mp) {
    MemPointDelta mpd;
    for (int i = 0; i < MemNumHeaps(); i++) {
        mpd.mHeapDelta[i] = mp.mHeapFree[i] - mHeapFree[i];
    }
    mpd.mPhysDelta = mPhysUsed - mp.mPhysUsed;
    return mpd;
}

MemPointDelta::MemPointDelta() { memset(this, 0, sizeof(MemPointDelta)); }

MemPointDelta &MemPointDelta::operator+=(const MemPointDelta &mpd) {
    for (int i = 0; i < MemNumHeaps(); i++) {
        mHeapDelta[i] += mpd.mHeapDelta[i];
    }
    mPhysDelta += mpd.mPhysDelta;
    return *this;
}

bool MemPointDelta::AnyGreaterThan(int i1) const {
    for (int i = 0; i < MemNumHeaps(); i++) {
        if (mHeapDelta[i] > i1) {
            return true;
        }
    }
    return (i1 < mPhysDelta) & 1;
}

const char *MemPointDelta::HeaderString(const char *s) {
    String st;
    for (int i = 0; i < MemNumHeaps(); i++) {
        if (i != 0) {
            st << ',';
        }
        st << MemHeapName(i);
        if (s != 0) {
            st << s;
        }
    }
    st << ",physical";
    if (s != 0) {
        st << s;
    }
    const char *c = MakeString("%s", st);
    return c;
}

const char *MemPointDelta::ToString(int divideBy) const {
    MILO_ASSERT(divideBy > 0, 0x5b);
    String st;
    for (int i = 0; i < MemNumHeaps(); i++) {
        if (i != 0) {
            st << ',';
        }
        st << mHeapDelta[i] / divideBy;
    }
    st << ',';
    st << mPhysDelta / divideBy;
    const char *c = MakeString("%s", st);
    return c;
}
