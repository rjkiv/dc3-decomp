#pragma once
#include "MemStats.h"
#include "obj/Data.h"
#include "utl/AllocInfo.h"
#include "utl/KeylessHash.h"
#include "utl/Str.h"
#include "utl/TextFileStream.h"
#include "utl/TextStream.h"
#include <cstdio>

struct MemDiffEntry {
    // total size: 0x48
    char name[59]; // offset 0x0, size 0x3B
    int alloc_diff; // offset 0x3C, size 0x4
    int bytes_diff; // offset 0x40, size 0x4
    int heap; // offset 0x44, size 0x4

    bool operator<(const MemDiffEntry &e) const {
        if (heap != e.heap) {
            return heap < e.heap;
        } else {
            return e.bytes_diff <= bytes_diff;
        }
    }
};

// size 0x1820c
class MemTracker {
public:
    MemTracker(int, int);
    const AllocInfo *GetInfo(void *) const;
    void Alloc(
        int requestedSize,
        int actualSize,
        const char *type,
        void *memory,
        signed char heap,
        bool pooled,
        unsigned char strat,
        const char *file,
        int line
    );
    void Free(void *);
    void CloseReport();
    void SetAllocInfoName(const char *);
    void StartLog(TextStream &);
    void StopLog();
    void Realloc(void *, int, int, void *);
    void HeapReport(TextStream &);
    void DiffDump(TextStream &);
    void ReportMemoryAlloc(const char *);
    void ReportMemoryUsage(const char *);
    void ReportMemoryUsageOverview(const char *);
    void Report(int, TextStream &);
    void SetSpew(bool spew) { mSpew = spew; }
    void SetReport(TextFileStream *s) { mReport = s; }
    signed char Heap() const { return mHeap; }
    bool GetHeapOnly() const { return mHeapOnly; }
    void SetHeapOnly(bool heapOnly) { mHeapOnly = heapOnly; }
    const String &GetTopLevelObjName() const { return mTopLevelObjectName; }
    void SetTopLevelObjName(const char *name) { mTopLevelObjectName = name; }
    HeapStats &HeapStatsAt(int idx) { return mHeapStats[idx]; }
    short GetTimeSlice() const { return mTimeSlice; }

    __forceinline void SetTopLevelFileName(const char *name) {
        mLastTopLevelFileName = mTopLevelFileName;
        mTopLevelFileName = name;
    }

    static void *operator new(unsigned int);
    static void operator delete(void *);
    static int SpitAllocInfo(TextStream *);
    static int SpitAllocInfo(FILE *);

private:
    void UpdateStats();
    void ColatedPrint(TextStream &, AllocInfo *, const char *);

    static DataNode SpitAllocInfo(DataArray *);

    AllocInfo **mHashMem; // 0x0
    KeylessHash<void *, AllocInfo *> *mHashTable; // 0x4
    short mTimeSlice; // 0x8
    HeapStats mHeapStats[16]; // 0xc
    BlockStatTable mHeapTypeStats[2]; // 0x14c
    BlockStatTable mPoolTypeStats[2]; // 0xc164
    int mCurStatTable; // 0x1817c
    AllocInfoVec mFreedInfos; // 0x18180
    TextStream *mLog; // 0x1818c
    TextFileStream *mReport; // 0x18190
    signed char mHeap; // 0x18194
    bool mHeapOnly; // 0x18195
    int mFreeSysMem; // 0x18198
    int mFreePhysMem; // 0x1819c
    bool mSpew; // 0x181a0
    // the last top file name from the file name stack
    String mLastTopLevelFileName; // 0x181a4
    // the file name at the top of the file name stack
    String mTopLevelFileName; // 0x181ac
    // the object name at the top of the object name stack
    String mTopLevelObjectName; // 0x181b4
    char mAllocInfoName[64]; // 0x181bc
    int unk182fc;
    int unk18200;
    int unk18204;
    int unk18208;
};

extern MemTracker *gMemTracker;

void MemTrackInit(int, int, bool);
bool MemTrackEnable(bool);
void MemTrackSpew(bool);
void MemTrackSetReportName(const char *);
