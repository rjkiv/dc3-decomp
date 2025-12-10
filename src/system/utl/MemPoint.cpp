#include "MemPoint.h"
#include "Memory.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/MemMgr.h"
#include "string.h"
#include "Str.h"

MemPoint::MemPoint(eInitType t) {
    if (t == kInitType1) {
        for (int i = 0; i < MemNumHeaps(); i++) {
            int a, b, c, d;
            MemFreeBlockStats(i, a, b, unk0[i], c, d);
        }
        unk40 = PhysicalUsage();
    } else {
        memset(unk0, 0, 0x44);
    }
}

MemPointDelta MemPoint::operator-(const MemPoint &mp) {
    MemPointDelta mpd;
    for (int i = 0; i < MemNumHeaps(); i++) {
        mpd.unk0[i] = mp.unk0[i] - unk0[i];
    }
    mpd.unk40 = unk40 - mp.unk40;
    return mpd;
}

MemPointDelta::MemPointDelta() { memset(unk0, 0, 0x44); }

MemPointDelta &MemPointDelta::operator+=(const MemPointDelta &mpd) {
    for (int i = 0; i < MemNumHeaps(); i++) {
        unk0[i] += mpd.unk0[i];
    }
    unk40 += mpd.unk40;
    return *this;
}

bool MemPointDelta::AnyGreaterThan(int i1) const {
    for (int i = 0; i < MemNumHeaps(); i++) {
        if (unk0[i] > i1) {
            return true;
        }
    }
    return (i1 < unk40) & 1;
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
        st << unk0[i] / divideBy;
    }
    st << ',';
    st << unk40 / divideBy;
    const char *c = MakeString("%s", st);
    return c;
}
