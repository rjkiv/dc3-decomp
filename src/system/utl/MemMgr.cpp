#include "utl/MemMgr.h"
#include "MemHeap.h"
#include "MemTracker.h"
#include "os/Memory.h"
#include "obj/Data.h"
#include "os/CritSec.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/Option.h"
#include "utl/PoolAlloc.h"
#include "utl/TextStream.h"
#include "xdk/XAPILIB.h"
#include <cstdlib>

#define MAX_HEAPS 16
#define MAX_BUF_THREADS 32

int MemHeapStack::sDefaultHeap = -1;
const char *gStlAllocName = "StlAlloc";

// these global offsets need fixing lol
static MemHeapStack gNullMemStack;
static MemHeapStack gThreadBuf[MAX_BUF_THREADS];
int gSingleHeap = 0;
static MemHeap gHeaps[MAX_HEAPS];
static bool sUnkB = false;
static bool gInitted = false;
bool gbUseLowestMip = false;
bool gInsideMemFunc = false;
static DWORD gCurThread = 0;
static DWORD gNumThreads = 0;
static int gNumHeaps = 0;
static int gCheckConsistency = 0;
static int gNewOperatorAlign = 0;
bool gStlAllocNameLookup = false;
CriticalSection *gMemLock = nullptr;
CriticalSection *gMemStackLock = nullptr;
std::vector<String> gUseLowestMipExceptions;

bool gMemoryUsageTest = false;
String gMemLogType;
DWORD gThreadIds[MAX_BUF_THREADS];

void *operator new(unsigned int size) {
    return MemAlloc(size, __FILE__, 0x5CF, "new", gNewOperatorAlign);
}

void *operator new[](unsigned int size) {
    return MemAlloc(size, __FILE__, 0x5E6, "new[]");
}

void operator delete(void *v) { MemFree(v, "unknown", 0, "unknown"); }
void operator delete[](void *v) { MemFree(v, "unknown", 0, "unknown"); }

void PhysDelta(const char *name) {
    static int gPhysicalUsage = -1;
    if (gPhysicalUsage == -1) {
        gPhysicalUsage = PhysicalUsage();
    }
    MEMORYSTATUS status;
    GlobalMemoryStatus(&status);
    TheDebug << name << " free:" << status.dwAvailPhys << " usage:" << PhysicalUsage()
             << " delta usage:" << PhysicalUsage() - gPhysicalUsage << "\n";
    gPhysicalUsage = PhysicalUsage();
}

bool MemUseLowestMip() { return gbUseLowestMip; }

int _GetFreePhysicalMemory() {
    int low = 0;
    int high = 0x40000000;
    int mid;
    do {
        mid = (high + low) / 2;
        void *ptr = XPhysicalAlloc(mid, -1, 0, 4);
        if (ptr) {
            low = mid;
            XPhysicalFree(ptr);
        } else {
            high = mid;
        }
    } while (low + 1 < high);
    return low;
}

int _GetFreeSystemMemory() {
    int low = 0;
    int high = 0x40000000;
    int mid;
    do {
        mid = (high + low) / 2;
        void *ptr = malloc(mid);
        if (ptr) {
            low = mid;
            free(ptr);
        } else {
            high = mid;
        }
    } while (low + 1 < high);
    return low;
}

int MemNumHeaps() { return gNumHeaps; }

void MemFree(void *mem, const char *file, int line, const char *name) {
    if (mem) {
        CritSecTracker tracker(gMemLock);
        int freeTotal = 0;
        int i;
        for (i = 0; i < gNumHeaps; i++) {
            freeTotal = gHeaps[i].Free((int *)mem);
            if (freeTotal)
                break;
        }
        if (i == gNumHeaps) {
            if (mem >= (void *)0xA0000000) {
                PhysicalFree(mem);
            } else {
                free(mem);
            }
        }
        if (gMemTracker) {
            MemTrackFree(mem);
            if (gMemTracker->GetHeapOnly()) {
                gMemTracker->HeapStatsAt((char)i).Free(freeTotal, freeTotal);
            }
        }
    }
}

void MemForceNewOperatorAlign(int align) { gNewOperatorAlign = align; }

void *MemTruncate(void *mem, int size, const char *file, int line, const char *name) {
    CritSecTracker tracker(gMemLock);
    if (!mem)
        return nullptr;
    else if (size == 0) {
        MemFree(mem);
        return nullptr;
    } else {
        int i;
        int allocSize = (size + 3) / sizeof(int);
        int i60;
        void *truncated = nullptr;
        for (i = 0; i < gNumHeaps; i++) {
            truncated = gHeaps[i].Truncate((int *)mem, allocSize, i60);
            if (truncated)
                break;
        }
        if (i == gNumHeaps) {
            truncated = realloc(mem, size);
            i60 = allocSize;
        }
        MemTrackRealloc(mem, size, i60 * sizeof(int), truncated);
        return truncated;
    }
}

const char *MemHeapName(int heap) {
    if (heap == -2)
        return "physical";
    else if (heap < 0)
        return "system";
    else
        return gHeaps[heap].Name();
}

int MemHeapSize(int heap) { return gHeaps[heap].SizeWords() * 4; }

int MemFindAddrHeap(void *addr) {
    for (int i = 0; i < gNumHeaps; i++) {
        if (addr >= gHeaps[i].Start() && addr < gHeaps[i].End()) {
            return i;
        }
    }
    return -2;
}

void MemPrint(int heapIdx, TextStream &stream, bool freeOnly) {
    CritSecTracker tracker(gMemLock);
    gHeaps[heapIdx].Print(stream, freeOnly);
}

void *MemOrPoolAlloc(int size, const char *file, int line, const char *name) {
    if (size == 0) {
        return nullptr;
    } else if (size > 0x80) {
        return MemAlloc(size, file, line, name);
    } else {
        return PoolAlloc(size, size, file, line, name);
    }
}

void MemOrPoolFree(int poolIdx, void *mem, const char *file, int line, const char *name) {
    if (mem) {
        if (poolIdx > 0x80) {
            MemFree(mem, file, line, name);
        } else {
            PoolFree(poolIdx, mem, file, line, name);
        }
    }
}

void MemOrPoolFreeSTL(
    int poolIdx, void *mem, const char *file, int line, const char *name
) {
    if (mem) {
        if (poolIdx > 0x80) {
            MemFree(mem, file, line, name);
        } else {
            PoolFree(poolIdx, mem, file, line, name);
        }
    }
}

void AddHeap(
    int heapNum,
    int bytes,
    const char *name,
    bool handle,
    int region,
    MemHeap::Strategy strat,
    int debugLevel,
    bool allowTemp
) {
    void *raw_mem = malloc(bytes);
    if (!raw_mem) {
        int max = 0x40000000;
        raw_mem = malloc(max);
        MILO_ASSERT(raw_mem, 0x32C);
        if (bytes > max) {
            MILO_LOG(
                "not enough memory for heap \"%s\". Requested: %d. Available: %d\n",
                name,
                bytes,
                max
            );
        }
        bytes = max;
    }
    gHeaps[heapNum].Init(
        name, gNumHeaps, (int *)raw_mem, bytes >> 2, handle, strat, debugLevel, allowTemp
    );
}

void AddHeap(int heapNum, int bytes, DataArray *arr) {
    Symbol handle("handle");
    Symbol region("region");
    Symbol debug("debug");
    Symbol strategy("strategy");
    Symbol allow_temp("allow_temp");
    const char *name = arr->Str(0);
    bool iHandle = false;
    arr->FindData(handle, iHandle, false);
    int iRegion = 0;
    arr->FindData(region, iRegion, false);
    bool iAllowTemp = true;
    arr->FindData(allow_temp, iAllowTemp, false);
    int iDebug = 0;
    arr->FindData(debug, iDebug, false);
    int iStrategy = 0;
    arr->FindData(strategy, iStrategy, false);
    AddHeap(
        heapNum,
        bytes,
        name,
        iHandle,
        iRegion,
        (MemHeap::Strategy)iStrategy,
        iDebug,
        iAllowTemp
    );
}

void *_MemAllocTemp(int size, const char *file, int line, const char *name, int align) {
    MemDoTempAllocations tmp;
    return MemAlloc(size, file, line, name, align);
}

void *MemOrPoolAllocSTL(int size, const char *file, int line, const char *name) {
    if (size == 0)
        return nullptr;
    else if (size > 0x80) {
        MemDoTempAllocations tmp;
        return MemAlloc(size, file, line, name, 0);
    } else {
        return PoolAlloc(size, size, file, line, name);
    }
}

void MemInit() {
    gMemLock = new CriticalSection();
    gMemStackLock = new CriticalSection();
    CritSecTracker tracker(gMemLock);
    bool disableMgr = false;
    bool enableTracking = false;
    bool noTrackImmediate = true;
    DataArray *cfg = SystemConfig("mem");
    cfg->FindData("check_consistency", gCheckConsistency);
    cfg->FindData("enable_tracking", enableTracking);
    cfg->FindData("disable_mgr", disableMgr);
    cfg->FindData("single_heap", gSingleHeap);
    cfg->FindData("no_track_immediate", noTrackImmediate, false);
    cfg->FindData("log_type", gMemLogType, false);
    cfg->FindData("use_lowest_mip", gbUseLowestMip, false);
    if (gbUseLowestMip) {
        DataArray *mipArr = cfg->FindArray("lowest_mip_exceptions", false);
        if (mipArr) {
            for (int i = 1; i < mipArr->Size(); i++) {
                String str(mipArr->Str(i));
                str.ToLower();
                gUseLowestMipExceptions.push_back(str);
            }
        }
    }
    int trackHeap = -1;
    cfg->FindData("track_heap", trackHeap, false);
    bool enableDejaReport;
    cfg->FindData("enable_deja_report", enableDejaReport, false);
    int trackedAllocs = -1;
    cfg->FindData("tracked_allocs", trackedAllocs, false);
    bool heapOnly = false;
    cfg->FindData("heap_only", heapOnly, false);
    bool spew = false;
    cfg->FindData("spew", spew, false);
    if (enableTracking) {
        MILO_LOG("MemTrack: free Memory %d\n", _GetFreeSystemMemory());
        MILO_LOG("MemTrack: free physical Memory %d\n", _GetFreePhysicalMemory());
    }
    DataArray *poolArr = cfg->FindArray("pool");
    if (UsingCD()) {
        if (cfg->FindArray("discReleaseHeaps")) {
            poolArr = cfg->FindArray("discReleasePool");
        }
    }
    PoolAllocInit(poolArr);
    if (!disableMgr) {
        void *mem = malloc(0x10000);
        DataArray *heapArr = cfg->FindArray("heaps");
        if (UsingCD()) {
            if (cfg->FindArray("discReleaseHeaps")) {
                heapArr = cfg->FindArray("discReleaseHeaps");
            }
        }
        if (gSingleHeap != 0) {
            gNumHeaps = 1;
        } else {
            gNumHeaps = heapArr->Size();
            MILO_ASSERT(gNumHeaps < MAX_HEAPS, 0x295);
        }
        int o12 = 0;
        Symbol size("size");
        AddHeap(
            heapArr->Size() - 1, 0x2500000, "tiny", false, 0, MemHeap::kFirstFit, 0, false
        );
        sUnkB = true;
        for (int i = heapArr->Size() - 1; i > 0; i--) {
            DataArray *heap = heapArr->Array(i);
            MILO_ASSERT(heap, 0x2B2);
            int heapSize = 0;
            heap->FindData(size, heapSize);
            if (gSingleHeap != 0) {
                o12 += heapSize;
                if (i == 1) {
                    AddHeap(0, o12, heap);
                }
                continue;
            }
            AddHeap(i - 1, heapSize, heap);
        }
        free(mem);
        MemHeapStack::sDefaultHeap = 0;
    }
    disableMgr = false;
    if (enableTracking) {
        MemTrackInit(trackHeap, trackedAllocs, heapOnly);
        MemTrackEnable(!noTrackImmediate);
        MemTrackSpew(spew);
        cfg->FindData("track_stl", gStlAllocNameLookup);
        MemDelta("-- MemTrackInit -- ", 0);
        if (OptionBool("memory_usage_test", false)) {
            gMemoryUsageTest = true;
            MILO_LOG("--- Executing Game in Memory Usage Test Mode ---\n");
            MemTrackSetReportName(OptionStr("budget_log", "mem_usage_test_x360.0000.csv"));
        }
        if (OptionBool("memory_alloc_test", false)) {
            MILO_LOG("--- Executing Game in Memory Alloc Test Mode ---\n");
            MemTrackSetReportName(OptionStr("budget_log", "alloc_test"));
        }
    }
    gInitted = true;
}

static bool HeapInitted() { return gInitted && gNumHeaps > 0; }

int MemAllocSize(void *mem) {
    CritSecTracker tracker(gMemLock);
    if (!mem)
        return 0;
    else {
        for (int i = 0; i < gNumHeaps; i++) {
            int size = gHeaps[i].AllocSize((int *)mem);
            if (size != 0) {
                return size;
            }
        }
        MILO_FAIL("Can't determine size of allocation.");
        return 0;
    }
}

void *MemResizeElem(
    void *&mem,
    int &totalSize,
    void *cutPoint,
    int cutLength,
    int insertLength,
    const char *file,
    int line,
    const char *name
) {
    void *old = mem;
    int prefixSize = (char *)cutPoint - (char *)mem;
    int suffixSize = 0;
    int newTotalSize = prefixSize;
    if (insertLength > -1) {
        suffixSize = (totalSize - newTotalSize) - cutLength;
        newTotalSize += suffixSize + insertLength;
    }
    if (newTotalSize != totalSize) {
        mem = MemAlloc(newTotalSize, file, line, name);
        totalSize = newTotalSize;
        if (prefixSize != 0) {
            memcpy(mem, old, prefixSize);
        }
        if (suffixSize != 0) {
            memcpy(
                (char *)mem + prefixSize + insertLength,
                (char *)cutPoint + cutLength,
                suffixSize
            );
        }
        MemFree(old, file, line, name);
    }
    return (char *)mem + prefixSize;
}

void *
MemRealloc(void *mem, int size, const char *file, int line, const char *name, int align) {
    CritSecTracker tracker(gMemLock);
    if (gNumHeaps != 0) {
        int memSize = MemAllocSize(mem);
        void *dst = MemAlloc(size, file, line, name, align);
        memcpy(dst, mem, size < memSize ? size : memSize);
        MemFree(mem);
        return dst;
    } else {
        void *dst = realloc(mem, size);
        MemTrackRealloc(mem, size, (size + 3) / 4, dst);
        return dst;
    }
}

MemHeapStack &ThreadMemStack(bool);

void MemPushHeap(int iHeap) {
    if (HeapInitted()) {
        MemHeapStack &s = ThreadMemStack(true);
        MILO_ASSERT_FMT(
            iHeap > kNoHeap && iHeap < gNumHeaps,
            "iHeap = %d, gNumHeaps=%d",
            iHeap,
            gNumHeaps
        );
        MILO_ASSERT(s.mSize + 1 < DIM(s.mStack), 0x1EA);
        s.mStack[s.mSize] = iHeap;
        s.mSize++;
    }
}

void MemPopHeap() {
    if (HeapInitted()) {
        MemHeapStack &s = ThreadMemStack(true);
        MILO_ASSERT(s.mSize > 0, 0x1f6);
        s.mSize--;
    }
}

void MemFreeBlockStats(
    int heapNum, int &i2, int &i3, int &numFreeBytes, int &i5, int &biggestFreeBlock
) {
    CritSecTracker tracker(gMemLock);
    MILO_ASSERT(heapNum < MAX_HEAPS, 0x154);
    gHeaps[heapNum].FreeBlockStats(i2, i3, numFreeBytes, i5, biggestFreeBlock);
}

int GetCurrentHeapNum() {
    MemHeapStack &s = ThreadMemStack(false);
    if (s.mSize != 0) {
        return s.mStack[s.mSize - 1];
    } else {
        return MemHeapStack::sDefaultHeap;
    }
}

void MemPushTemp() {
    if (HeapInitted()) {
        MemHeapStack &s = ThreadMemStack(true);
        s.mTempRefs++;
    }
}

void MemPopTemp() {
    if (HeapInitted()) {
        MemHeapStack &s = ThreadMemStack(true);
        MILO_ASSERT(s.mTempRefs > 0, 0x209);
        s.mTempRefs--;
    }
}

bool MemUseLowestMipException(const char *cc1) {
    String str(cc1);
    str.ToLower();
    FOREACH (it, gUseLowestMipExceptions) {
        if (strstr(str.c_str(), it->c_str())) {
            return true;
        }
    }
    return false;
}

int MemFindHeap(const char *cc) {
    for (int i = 0; i < gNumHeaps; i++) {
        if (gHeaps[i].Name() && streq(gHeaps[i].Name(), cc)) {
            return i;
        }
    }
    if (streq("char", cc)) {
        return 0;
    } else if (streq("physical", cc)) {
        return -2;
    } else if (gSingleHeap) {
        return 0;
    } else {
        if (HeapInitted()) {
            MILO_FAIL("could not find heap \"%s\"", cc);
        }
        return -1;
    }
}

void MemDelta(const char *msg, int heapNum) {
    int lfrag = 0;
    int rfrag = 0;
    int numFree = 0;
    int i5 = 0;
    int largest = 0;
    MemFreeBlockStats(heapNum, lfrag, rfrag, numFree, i5, largest);

    static int sFreeHeaps[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

    if (sFreeHeaps[heapNum] == -1) {
        sFreeHeaps[heapNum] = numFree;
    }
    int delta = sFreeHeaps[heapNum] - numFree;
    TheDebug << msg << " lfrag:" << lfrag << " rfrag:" << rfrag << " largest:" << largest
             << " free:" << numFree << " fragmentation:" << numFree - largest
             << " delta:" << delta << "\n";
    sFreeHeaps[heapNum] = numFree;
}
