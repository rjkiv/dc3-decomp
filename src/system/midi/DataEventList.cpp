#include "midi/DataEventList.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include "utl/TextStream.h"
#include "utl/TimeConversion.h"
#include <algorithm>

namespace {
    struct EventTimeComp {
        bool operator()(const DataEvent &e, const float &f) const {
            return e.start < f ? true : false;
        }
    };
    struct EventTimeCompComp {
        bool operator()(const DataEventList::CompEv &e, const float &f) const {
            return e.start < f ? true : false;
        }
    };
}

float *DataEventList::EndPtr(int index) {
    MILO_ASSERT(index < mSize, 0xC0);
    if (mElement < 0)
        return &mEvents[index].end;
    else
        return &mComps[index].end;
}

void DataEventList::Invert(float f) {
    if (mElement < 0) {
        for (int i = 0; i < mSize; i++) {
            DataEvent &event = mEvents[i];
            float tmp = event.end;
            event.end = event.start;
            event.start = f;
            f = tmp;
        }
    } else {
        for (int i = 0; i < mSize; i++) {
            CompEv &event = mComps[i];
            float tmp = event.end;
            event.end = event.start;
            event.start = f;
            f = tmp;
        }
    }
}

void DataEventList::SecOffset(float fff) {
    float mult = fff * 1000.0f;
    if (mElement < 0) {
        for (int i = 0; i < mSize; i++) {
            DataEvent &event = mEvents[i];
            event.start = MsToBeat(mult + BeatToMs(event.start));
            event.end = MsToBeat(mult + BeatToMs(event.end));
        }
    } else {
        for (int i = 0; i < mSize; i++) {
            CompEv &event = mComps[i];
            event.start = MsToBeat(mult + BeatToMs(event.start));
            event.end = MsToBeat(mult + BeatToMs(event.end));
        }
    }
}

void DataEventList::Compress(DataArray *arr, int i) {
    mElement = i;
    mTemplate.SetMsg(arr);
    // i hate this
    DataNode *n = &arr->Node(i);
    mValue = reinterpret_cast<int *>(n);
}

int DataEventList::FindStartFromBack(float start) const {
    int ret = mSize - 1;
    if (mElement < 0) {
        for (; ret >= 0; ret--) {
            if (start >= mEvents[ret].start) {
                break;
            }
        }
    } else {
        for (; ret >= 0; ret--) {
            if (start >= mComps[ret].start) {
                break;
            }
        }
    }
    return ret;
}

const DataEvent &DataEventList::Event(int idx) const {
    if (mElement >= 0) {
        const DataEventList::CompEv &ev = mComps[idx];
        DataEvent &temp = const_cast<DataEventList *>(this)->mTemplate;
        int &val = *mValue;
        temp.start = ev.start;
        temp.end = ev.end;
        val = ev.value;
        return temp;
    } else {
        return mEvents[idx];
    }
}

const DataEvent *DataEventList::NextEvent(float f) {
    if (mCurIndex >= mSize)
        return 0;
    if (mElement >= 0) {
        if (f < mComps[mCurIndex].start)
            return 0;
    } else if (f < mEvents[mCurIndex].start)
        return 0;
    return &Event(mCurIndex++);
}

void DataEventList::Reset(float f) {
    EventTimeCompComp compcomp;
    EventTimeComp comp;

    if (mElement >= 0) {
        CompEv *lower = std::lower_bound(mComps.begin(), mComps.end(), f, compcomp);
        mCurIndex = lower - mComps.begin();
    } else {
        DataEvent *lower = std::lower_bound(mEvents.begin(), mEvents.end(), f, comp);
        mCurIndex = lower - mEvents.begin();
    }
}

DataEventList::DataEventList()
    : mCurIndex(0), mSize(0), mElement(-1), mCompType(kDataUnhandled), mValue(0) {}

DataEventList::~DataEventList() {}

void DataEventList::Clear() {
    mCurIndex = 0;
    mSize = 0;
    ClearAndShrink(mComps);
    ClearAndShrink(mEvents);
}

void DataEventList::Compact() {
    if (mElement < 0) {
        MILO_ASSERT(mComps.empty(), 0x104);
        TrimExcess(mEvents);
    } else {
        MILO_ASSERT(mEvents.empty(), 0x109);
        TrimExcess(mComps);
    }
}

void DataEventList::InsertEvent(float start, float end, const DataNode &node, int idx) {
    if (mElement < 0) {
        if (mSize == 0)
            mEvents.reserve(32);
        DataEvent d(start, end, node.Array());
        mEvents.insert(mEvents.begin() + idx, d);
    } else {
        CompEv event;
        event.start = start;
        event.end = end;
        event.value = node.IntValue();
        if (mSize == 0) {
            mComps.reserve(32);
            mCompType = node.Type();
            mTemplate.Msg()->Node(mElement) = node;
            MILO_ASSERT(mCompType == kDataSymbol || mCompType == kDataInt, 0x43);
        } else if (mCompType != node.Type()) {
            String str;
            node.Print(str, false, 0);
            MILO_NOTIFY(
                "Trying to add event %s but mCompType is %s, ignoring",
                str,
                mCompType == kDataInt ? "kDataInt" : "kDataSymbol"
            );
            return;
        }
        mComps.insert(mComps.begin() + idx, event);
    }
    mSize++;
}

void DataEventList::Print(TextStream &ts) const {
    for (int i = 0; i < mSize; i++) {
        const DataEvent &e = Event(i);
        ts << e.start << " " << e.end << " " << e.Msg() << "\n";
    }
}
