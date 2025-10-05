#pragma once
#include "obj/Data.h"
#include "utl/Str.h"
#include <map>

class DataPoint {
private:
    Symbol mType; // 0x0
    std::map<Symbol, DataNode> mNameValPairs; // 0x4

public:
    DataPoint();
    DataPoint(const char *);

    void AddPair(const char *, DataNode);
    void AddPair(Symbol, DataNode);
    void ToJSON(String &) const;
};

typedef void DataPointRecordFunc(DataPoint &);

class DataPointMgr {
private:
    DataPointRecordFunc *mDataPointRecorder; // 0x0
    DataPointRecordFunc *mDebugDataPointRecorder; // 0x4
    const char *mHostName; // 0x8
    const char *mApp; // 0xc
    const char *mProject; // 0x10
    int mVersion; // 0x14
    String unk18; // 0x18
    String unk20; // 0x20
    String unk28; // 0x28

public:
    DataPointMgr();
    ~DataPointMgr();

    DataPointRecordFunc *SetDataPointRecorder(DataPointRecordFunc *);
    void RecordDataPoint(DataPoint &);
    void RecordDebugDataPoint(DataPoint &);
    void Init();
};

extern DataPointMgr &TheDataPointMgr;

template <class N1, class V1>
void SendDataPoint(const char *type, N1 name1, V1 value1) {
    DataPoint point(type);
    point.AddPair(name1, value1);
    TheDataPointMgr.RecordDataPoint(point);
}

template <class N1, class V1, class N2, class V2>
void SendDataPoint(const char *type, N1 name1, V1 value1, N2 name2, V2 value2) {
    DataPoint point(type);
    point.AddPair(name1, value1);
    point.AddPair(name2, value2);
    TheDataPointMgr.RecordDataPoint(point);
}

template <class N1, class V1, class N2, class V2, class N3, class V3>
void SendDataPoint(
    const char *type,
    // clang-format off: looks nicer this way
    N1 name1, V1 value1,
    N2 name2, V2 value2,
    N3 name3, V3 value3
    // clang-format on
) {
    DataPoint point(type);
    point.AddPair(name1, value1);
    point.AddPair(name2, value2);
    point.AddPair(name3, value3);
    TheDataPointMgr.RecordDataPoint(point);
}

template <class N1, class V1, class N2, class V2, class N3, class V3, class N4, class V4>
void SendDataPoint(
    const char *type,
    // clang-format off
    N1 name1, V1 value1,
    N2 name2, V2 value2,
    N3 name3, V3 value3,
    N4 name4, V4 value4
    // clang-format on
) {
    DataPoint point(type);
    point.AddPair(name1, value1);
    point.AddPair(name2, value2);
    point.AddPair(name3, value3);
    point.AddPair(name4, value4);
    TheDataPointMgr.RecordDataPoint(point);
}

// clang-format off
template <
    class N1, class V1,
    class N2, class V2,
    class N3, class V3,
    class N4, class V4,
    class N5, class V5
>
void SendDataPoint(
    const char *type,
    N1 name1, V1 value1,
    N2 name2, V2 value2,
    N3 name3, V3 value3,
    N4 name4, V4 value4,
    N5 name5, V5 value5
    // clang-format on
) {
    DataPoint point(type);
    point.AddPair(name1, value1);
    point.AddPair(name2, value2);
    point.AddPair(name3, value3);
    point.AddPair(name4, value4);
    point.AddPair(name5, value5);
    TheDataPointMgr.RecordDataPoint(point);
}

// clang-format off
template <
    class N1, class V1,
    class N2, class V2,
    class N3, class V3,
    class N4, class V4,
    class N5, class V5,
    class N6, class V6
>
void SendDataPoint(
    const char *type,
    N1 name1, V1 value1,
    N2 name2, V2 value2,
    N3 name3, V3 value3,
    N4 name4, V4 value4,
    N5 name5, V5 value5,
    N6 name6, V6 value6
    // clang-format on
) {
    DataPoint point(type);
    point.AddPair(name1, value1);
    point.AddPair(name2, value2);
    point.AddPair(name3, value3);
    point.AddPair(name4, value4);
    point.AddPair(name5, value5);
    point.AddPair(name6, value6);
    TheDataPointMgr.RecordDataPoint(point);
}

// clang-format off
template <
    class N1, class V1,
    class N2, class V2,
    class N3, class V3,
    class N4, class V4,
    class N5, class V5,
    class N6, class V6,
    class N7, class V7
>
void SendDataPoint(
    const char *type,
    N1 name1, V1 value1,
    N2 name2, V2 value2,
    N3 name3, V3 value3,
    N4 name4, V4 value4,
    N5 name5, V5 value5,
    N6 name6, V6 value6,
    N7 name7, V7 value7
    // clang-format on
) {
    DataPoint point(type);
    point.AddPair(name1, value1);
    point.AddPair(name2, value2);
    point.AddPair(name3, value3);
    point.AddPair(name4, value4);
    point.AddPair(name5, value5);
    point.AddPair(name6, value6);
    point.AddPair(name7, value7);
    TheDataPointMgr.RecordDataPoint(point);
}
