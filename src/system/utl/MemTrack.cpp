#include "utl/MemTrack.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "utl/AllocInfo.h"
#include "utl/MemMgr.h"
#include "utl/MemTracker.h"
#include "utl/PoolAlloc.h"
#include "utl/TextFileStream.h"

AllocInfo *gAllocInfoHeap;
MemTracker *gMemTracker;
bool gMemTrackerTracking;
bool gMemoryUsageTest;
// HeapTracker* gHeapTracker;
int gNumDiffs;
TextFileStream *gLog;

void StopLog() {
    if (gLog) {
        RELEASE(gLog);
    }
}

bool MemTrackEnable(bool enable) {
    bool old = gMemTrackerTracking;
    gMemTrackerTracking = enable;
    return old;
}

void MemTrackSpew(bool spew) {
    if (gMemTracker) {
        gMemTracker->SetSpew(spew);
    }
}

void MemTrackSetReportName(const char *name) {
    if (gMemTracker) {
        gMemTracker->SetReport(new TextFileStream(name, false));
        gMemTracker->SetAllocInfoName(name);
    }
}

void MemTrackReportMemoryAlloc(const char *name) {
    if (gMemTracker) {
        gMemTracker->ReportMemoryAlloc(name);
    }
}

void MemTrackReportMemoryUsage(const char *name) {
    if (gMemTracker) {
        gMemTracker->ReportMemoryUsage(name);
    }
}

void MemTrackReportClose(const char *name) {
    if (gMemTracker) {
        gMemTracker->ReportMemoryUsageOverview(name);
        gMemTracker->CloseReport();
    }
}

void MemTrackAlloc(
    int req,
    int act,
    const char *type,
    void *mem,
    bool pooled,
    unsigned char strat,
    const char *file,
    int line
) {
    if (gMemTracker && gMemTrackerTracking) {
        CritSecTracker tracker(gMemLock);
        int heap = GetCurrentHeapNum();
        if (mem >= (void *)0xA0000000) {
            heap = MemNumHeaps();
        }
        gMemTracker->Alloc(req, act, type, mem, heap, pooled, strat, file, line);
    }
}

void MemTrackFree(void *mem) {
    if (gMemTracker) {
        CritSecTracker tracker(gMemLock);
        gMemTracker->Free(mem);
    }
}

void MemTrackRealloc(void *key, int req, int act, void *mem) {
    if (gMemTracker) {
        CritSecTracker tracker(gMemLock);
        gMemTracker->Realloc(key, req, act, mem);
    }
}

const AllocInfo *MemTrackGetInfo(void *key) {
    if (gMemTracker) {
        CritSecTracker tracker(gMemLock);
        return gMemTracker->GetInfo(key);
    } else {
        return nullptr;
    }
}

void *DebugHeapAlloc(int size) { return malloc(size); }
void DebugHeapFree(void *mem) { free(mem); }

void MemDeltaFullReport() {
    for (int i = 0; i < MemNumHeaps(); i++) {
        MemDelta("", i);
    }
    PhysDelta("");
}

void StartLog(const char *base) {
    char buffer[64];
    if (gLog) {
        StopLog();
    }
    MILO_ASSERT(!gLog, 0x5B);
    if (strstr(base, "diff")) {
        gNumDiffs++;
    }
    int num = gNumDiffs;
    while (true) {
        MILO_ASSERT(strlen( base ) < 55, 0x68);
        strcpy(buffer, MakeString("%s_%03i.txt", base, num));
        num++;
        File *file = NewFile(buffer, 0x10001);
        if (!file)
            break;
        delete file;
    }
    MILO_LOG("writing file %s\n", buffer);
    gLog = new TextFileStream(buffer, false);
}

void MemTrackReport(int i1, bool b2) {
    if (gMemTracker) {
        CritSecTracker tracker(gMemLock);
        if (b2) {
            StartLog("mem_report");
            gMemTracker->Report(i1, *gLog);
            PoolReport(*gLog);
            StopLog();
            StartLog("mem_diff");
            gMemTracker->DiffDump(*gLog);
            StopLog();
        } else {
            gMemTracker->DiffDump(TheDebug);
        }
    }
}

void MemTrackHeapDump(bool freeOnly) {
    CritSecTracker tracker(gMemLock);
    StartLog("mem_dump");
    *gLog << "(executable " << TheSystemArgs.front() << ")\n";
    *gLog << "(data\n";
    for (int i = 0; i < MemNumHeaps(); i++) {
        if (gMemTracker) {
            if (gMemTracker->Heap() == -1 || gMemTracker->Heap() == i) {
                MemPrint(i, *gLog, freeOnly);
            }
        }
    }
    *gLog << ")\n";
    StopLog();
}

DataNode MemTrackReportDF(DataArray *) {
    MemTrackReport(1000, true);
    return 0;
}

DataNode MemTrackHeapDumpDF(DataArray *) {
    MemTrackHeapDump(false);
    return 0;
}

DataNode MemTrackLogDF(DataArray *a) {
    if (a->Int(1) == 1) {
        StartLog("mem_log");
        gMemTracker->StartLog(*gLog);
    } else {
        gMemTracker->StopLog();
        StopLog();
    }
    return 0;
}
