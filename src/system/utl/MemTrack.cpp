#include "utl/MemTrack.h"
#include "obj/DataFunc.h"
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

static AllocInfo *gAllocInfoHeap = nullptr;
MemTracker *gMemTracker = nullptr;
bool gMemTrackerTracking = false;
bool gMemoryUsageTest = false;
class HeapTracker *gHeapTracker = nullptr;

static int gNumDiffs = 0;
static TextFileStream *gLog = nullptr;

#define STACK_SIZE 64

static char *s_MemTrackObjectName[STACK_SIZE + 1] = { 0 };
static int s_MemTrackObjectNameStackPos = 0;
static char *s_MemTrackFileName[STACK_SIZE + 1] = { 0 };
static int s_MemTrackFileNameStackPos = 0;

String gMemTrackSourceFile;
String gMemTrackSourceObject;

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
    int num = gNumDiffs;
    if (strstr(base, "diff")) {
        gNumDiffs++;
    }
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

void MemTrackInit(int heap, int numAllocs, bool heapOnly) {
    CritSecTracker tracker(gMemLock);
    MILO_ASSERT(!gMemTracker, 0x82);
    if (heapOnly) {
        numAllocs = 1;
    }
    gMemTracker = new MemTracker(heap, numAllocs);
    gMemTracker->SetHeapOnly(heapOnly);
    gAllocInfoHeap = (AllocInfo *)malloc(numAllocs * sizeof(AllocInfo));
    MILO_ASSERT(gAllocInfoHeap, 0x89);
    AllocInfo::SetPoolMemory(gAllocInfoHeap, numAllocs * sizeof(AllocInfo));
    DataRegisterFunc("heap_report", MemTrackReportDF);
    DataRegisterFunc("heap_dump", MemTrackHeapDumpDF);
    DataRegisterFunc("mem_log", MemTrackLogDF);
    MemTrackReport(0, false);
    AllocInfoInit();
    for (int i = 0; i <= (int)DIM(s_MemTrackFileName) - 1; i++) {
        s_MemTrackFileName[i] =
            (char *)MemAlloc(0x80, __FILE__, 0x9a, "MemTrackStack", 0);
        memset(s_MemTrackFileName[i], 0, 0x80);
        s_MemTrackObjectName[i] =
            (char *)MemAlloc(0x80, __FILE__, 0x9c, "MemTrackStack", 0);
        memset(s_MemTrackObjectName[i], 0, 0x80);
    }
}

void BeginMemTrackObjectName(const char *cc) {
    if (gMemTracker) {
        s_MemTrackObjectNameStackPos++;
        MILO_ASSERT(s_MemTrackObjectNameStackPos <= STACK_SIZE, 0xBE);
        strncpy(
            s_MemTrackObjectName[s_MemTrackObjectNameStackPos],
            gMemTracker->StrUnk181b4().c_str(),
            0x80
        );
        s_MemTrackObjectName[s_MemTrackObjectNameStackPos][0x7f] = '\0';
        static bool sNavPlayerToggle = false;
        if (streq(cc, "flow/nav_player.milo")) {
            sNavPlayerToggle = !sNavPlayerToggle;
        }
        gMemTracker->SetStrUnk181b4(cc);
    }
}
