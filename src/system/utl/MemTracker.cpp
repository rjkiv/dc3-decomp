#include "utl/MemTracker.h"
#include "AllocInfo.h"
#include "MemMgr.h"
#include "MemTrack.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "os/Memory.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/KeylessHash.h"
#include "math/Sort.h"
#include "utl/MakeString.h"
#include "utl/MemMgr.h"
#include "utl/MemStats.h"
#include "utl/Symbol.h"
#include "utl/TextFileStream.h"
#include "utl/TextStream.h"

bool gMemTrackerTracking;
String gMemLogType;

bool StackLess(AllocInfo *const &a1, AllocInfo *const &a2) {
    return a1->StackCompare(*a2) < 0;
}

int HashKey(void *ptr, int size) {
    MILO_ASSERT((uint(ptr) & 7) == 0, 0x25);
    return (uint(ptr) / 8) % size;
}

void DiffTblReport(
    const char *caption, BlockStatTable &tbl0, BlockStatTable &tbl1, TextStream &stream
) {
    tbl0.SortByName();
    tbl1.SortByName();
    int idx0 = 0;
    int idx1 = 0;
    int stats0 = tbl0.GetNumStats();
    int stats1 = tbl1.GetNumStats();
    std::vector<MemDiffEntry> entries;
    entries.reserve(stats0 + stats1);
    while (idx0 < stats0 && idx1 < stats1) {
        BlockStat &st0 = tbl0.GetBlockStat(idx0);
        BlockStat &st1 = tbl1.GetBlockStat(idx1);
        int name_cmp = strcmp(st0.mName, st1.mName);
        int alloc0, alloc1;
        int req0, req1;
        int heap;
        const char *name = st1.mName;
        if (name_cmp < 0) {
            alloc0 = st0.mNumAllocs;
            alloc1 = 0;
            req0 = st0.mSizeReq;
            req1 = 0;
            name = st0.mName;
            heap = st0.mHeap;
            idx0++;
        } else if (name_cmp > 0) {
            alloc0 = 0;
            alloc1 = st1.mNumAllocs;
            req0 = 0;
            req1 = st1.mSizeReq;
            heap = st1.mHeap;
            idx1++;
        } else {
            alloc0 = st0.mNumAllocs;
            alloc1 = st1.mNumAllocs;
            req0 = st0.mSizeReq;
            req1 = st1.mSizeReq;
            heap = st1.mHeap;
            idx0++;
            idx1++;
        }
        int alloc_diff = alloc0 - alloc1;
        int bytes_diff = req0 - req1;
        if (alloc_diff != 0 || bytes_diff != 0) {
            MemDiffEntry entry;
            strncpy(entry.name, name, sizeof(entry.name) - 1);
            entry.name[sizeof(entry.name) - 1] = '\0';
            entry.alloc_diff = alloc_diff;
            entry.bytes_diff = bytes_diff;
            entry.heap = heap;
            entries.push_back(entry);
        }
    }
    int totalBytes = 0;
    int totalAlloc = 0;
    std::sort(entries.begin(), entries.end());
    stream << MakeString("%-62s %8s %8s\n", caption, "Num", "Bytes");
    int lastHeap = -2;
    FOREACH (it, entries) {
        MemDiffEntry &cur = *it;
        if (cur.heap != lastHeap) {
            stream << MakeString(" HEAP %d ------------------\n", cur.heap);
            lastHeap = cur.heap;
        }
        totalBytes += cur.bytes_diff;
        totalAlloc += cur.alloc_diff;
        stream
            << MakeString("  %-60s %8d %8d\n", cur.name, cur.alloc_diff, cur.bytes_diff);
    }
    stream << MakeString(" %-61s %8d %8d\n\n", "TOTAL ------", totalAlloc, totalBytes);
}

MemTracker::MemTracker(int heap, int numAllocs)
    : mHashMem(nullptr), mHashTable(nullptr), mTimeSlice(0), mCurStatTable(0),
      mFreedInfos(DebugHeapAlloc(numAllocs * 4), numAllocs), mLog(0), mReport(0),
      mHeap(heap) {
    int hashSize = heap * 2;
    mHashMem = (AllocInfo **)DebugHeapAlloc(numAllocs * 8);
    MILO_ASSERT(mHashMem, 0x4E);
    mHashTable = new KeylessHash<void *, AllocInfo *>(
        hashSize, (AllocInfo *)0, (AllocInfo *)-1, mHashMem
    );
    mFreeSysMem = _GetFreeSystemMemory();
    mFreePhysMem = _GetFreePhysicalMemory();
    DataRegisterFunc("spit_alloc_info", SpitAllocInfo);
    DataRegisterFunc("sai", SpitAllocInfo);
}

void *MemTracker::operator new(unsigned int size) { return DebugHeapAlloc(size); }
void MemTracker::operator delete(void *mem) { DebugHeapFree(mem); }

const AllocInfo *MemTracker::GetInfo(void *info) const {
    AllocInfo **found = mHashTable->Find(info);
    if (found) {
        return *found;
    } else
        return nullptr;
}

void MemTracker::Alloc(
    int requestedSize,
    int actualSize,
    const char *type,
    void *memory,
    signed char heap,
    bool pooled,
    unsigned char strat,
    const char *file,
    int line
) {
    if (!gMemTrackerTracking)
        return;
    MILO_ASSERT(type, 0x6D);
    if (mHeap != -1 && heap != mHeap) {
        return;
    }
    gMemTrackerTracking = false;
    AllocInfo::bPrintCsv = true;
    if (!mHeapOnly) {
        AllocInfo *info = new AllocInfo(
            requestedSize,
            actualSize,
            type,
            memory,
            heap,
            pooled,
            strat,
            file,
            line,
            mTopLevelFileName,
            mTopLevelObjectName
        );
        mHashTable->Insert(info);
        if (pooled || gMemLogType != gNullStr || gMemLogType == type) {
            if (pooled || mHeap != -1 && heap != mHeap) {
                if (mLog) {
                    *mLog << " ((com new) " << "(mem " << memory << ") " << info << ")\n";
                }
                if (mSpew) {
                    TheDebug << "::Alloc::" << info->mType << " Allocated "
                             << info->mActSize << " Requested " << info->mReqSize
                             << " Address " << info->mMem << " Heap " << info->mHeap
                             << mTopLevelFileName.c_str() << ":"
                             << mTopLevelObjectName.c_str() << "\n";
                }
            }
        } else {
            // if !mLog goto above
            *mLog << " new, ";
            info->PrintCsv(*mLog);
            *mLog << "\n";
        }
    }
    if (!pooled) {
        mHeapStats[heap].Alloc(actualSize, requestedSize);
    }
    gMemTrackerTracking = true;
}

void MemTracker::Free(void *mem) {
    AllocInfo **found = mHashTable->Find(mem);
    if (found) {
        AllocInfo *info = *found;
        info->Validate();
        if (mLog && !info->mPooled && (mHeap == -1 || info->mHeap == mHeap)
            && info->mStrat == 0) {
            *mLog << " ((com free) " << "(" << mem << ") " << *info << ")\n";
        }
        if (!info->mPooled) {
            mHeapStats[info->mHeap].Free(info->mActSize, info->mReqSize);
        }
        mHashTable->Remove(found);
        if (info->mTimeSlice == mTimeSlice) {
            delete info;
        } else {
            mFreedInfos.push_back(info);
        }
    }
}

void MemTracker::ColatedPrint(TextStream &ts, AllocInfo *info, const char *com) {
    ts << "  ((com " << com << ") (rep " << 1 << " ) " << *info << ")\n";
}

void MemTracker::CloseReport() {
    if (mReport) {
        MemNumHeaps();
        TextStream &ts = *mReport;
        ts << "\n";
        ts << "\n";
        ts << "Category,CategoryName,Column,Budget,BudgetType,AlwaysShow,Tooltip\n";
        ts << "column_info,overview,Mode,0,0,1,notes\n";
        ts << "column_info,overview,MainPeak,0,0,1,notes\n";
        ts << "column_info,overview,MainAlloc,0,0,1,notes\n";
        ts << "column_info,overview,MainLargest,0,1,0,notes\n";
        ts << "column_info,overview,CharPeak,0,0,1,notes\n";
        ts << "column_info,overview,CharAlloc,0,0,1,notes\n";
        ts << "column_info,overview,CharLargest,0,1,0,notes\n";
        ts << "column_info,overview,PhysPeak,0,0,1,notes\n";
        ts << "column_info,overview,PhysAlloc,0,0,1,notes\n";
        ts << "column_info,overview,PhysLargest,0,0,1,notes\n";
        ts << "column_info,base,heap,-1.0,-1,1,heap name\n";
        ts << "column_info,base,free,0,0,0,bytes free in heap\n";
        ts << "column_info,base,biggest,0,0,1,size of largest free block\n";
        ts << "column_info,base,lfrags,0,0,0,fragmentation count at low end of memory\n";
        ts << "column_info,base,requested,0,0,0,amount of memory actually requested\n";
        ts << "column_info,base,allocated,0,0,1,amount of memory actually allocated\n";
        ts << "column_info,base,peak,0,0,1,memory high water mark\n";
        ts << "column_info,game,heap,-1.0,-1,1,heap name\n";
        ts << "column_info,game,free,0,0,0,bytes free in heap\n";
        ts << "column_info,game,biggest,0,0,1,size of largest free block\n";
        ts << "column_info,game,lfrags,0,0,0,fragmentation count at low end of memory\n";
        ts << "column_info,game,requested,0,0,0,amount of memory actually requested\n";
        ts << "column_info,game,allocated,0,0,1,amount of memory actually allocated\n";
        ts << "column_info,game,peak,0,0,1,memory high water mark\n";
        ts << "\n";
        ts << "Category,CategoryName\n";
        ts << "category_info,game\n";
        ts << "category_info,base\n";
        ts << "\nDone\n";
        mReport->File().Flush();
        RELEASE(mReport);
    }
}

void MemTracker::SetAllocInfoName(const char *name) {
    Hx_snprintf(mAllocInfoName, 64, "%s", name);
}

void MemTracker::StartLog(TextStream &ts) {
    if (mLog) {
        StopLog();
    }
    MILO_ASSERT(!mLog, 0x113);
    mLog = &ts;
    *mLog << "(elf " << TheSystemArgs.front() << ")\n";
    *mLog << "(data\n";
}

void MemTracker::StopLog() {
    if (mLog) {
        *mLog << ")";
        mLog = nullptr;
    }
}

void MemTracker::Realloc(void *key, int reqSize, int actualSize, void *mem) {
    AllocInfo **found = mHashTable->Find(key);
    if (found) {
        AllocInfo *info = *found;
        info->Validate();
        bool validHeap = mHeap == -1 || info->mHeap == mHeap;
        MILO_ASSERT(validHeap, 0xF6);
        if (reqSize == -1) {
            reqSize = info->mReqSize;
        }
        if (actualSize == -1) {
            actualSize = info->mActSize;
        }
        signed char heap = info->mHeap;
        unsigned char strat = info->mStrat;
        const char *type = info->mType;
        MILO_ASSERT(info->mPooled == 0, 0x100);
        Free(key);
        Alloc(reqSize, actualSize, type, mem, heap, false, strat, __FILE__, 0x102);
    }
}

void MemTracker::HeapReport(TextStream &ts) {
    int max = MemNumHeaps() + 1;
    for (int i = 0; i < max; i++) {
        HeapStats &curStats = mHeapStats[i];
        ts << MakeString("\n*** FREE LIST for heap #%d ***\n", i);
        if (i == MemNumHeaps()) {
            ts << MakeString("  Heap name          = %14s\n", "physical");
            ts << MakeString("  Heap size          = %14d\n", mFreePhysMem);
            ts << MakeString(
                "  Num Free Bytes     = %14d\n", mFreePhysMem - PhysicalUsage()
            );
            ts << MakeString("  Biggest Free Block = %14d\n", _GetFreePhysicalMemory());
            ts << MakeString("  Num Free Blocks    = %14s\n", "N/A");
        } else {
            int i1, i2, i3, i4, i5;
            MemFreeBlockStats(i, i1, i2, i3, i4, i5);
            ts << MakeString("  Heap name          = %14s\n", MemHeapName(i));
            ts << MakeString("  Heap size          = %14d\n", MemHeapSize(i));
            ts << MakeString("  Num Free Bytes     = %14d\n", i3);
            ts << MakeString("  Biggest Free Block = %14d\n", i5);
            ts << MakeString("  lFrags             = %14d\n", i1);
        }
        ts << MakeString("  Num Allocs         = %14d\n", curStats.mTotalNumAllocs);
        ts << MakeString("  Bytes Requested    = %14d\n", curStats.mTotalReqSize);
        ts << MakeString("  Bytes Allocated    = %14d\n", curStats.mTotalActSize);
        ts << MakeString("  Peak Num Allocs    = %14d\n", curStats.mMaxNumAllocs);
        ts << MakeString("  Peak Bytes Alloc'd = %14d\n", curStats.mMaxActSize);
    }
}

void MemTracker::UpdateStats() {
    mPoolTypeStats[mCurStatTable].Clear();
    mHeapTypeStats[mCurStatTable].Clear();
    for (auto it = mHashTable->Begin(); it != nullptr; it = mHashTable->Next(it)) {
        AllocInfo *info = *it;
        if (info->mPooled) {
            mPoolTypeStats[mCurStatTable].Update(
                info->mType, info->mHeap, info->mReqSize, info->mActSize
            );
        } else {
            mHeapTypeStats[mCurStatTable].Update(
                info->mType, info->mHeap, info->mReqSize, info->mActSize
            );
        }
    }
}

DataNode MemTracker::SpitAllocInfo(DataArray *a) {
    int ret = 1;
    if (a && a->Size() > 1) {
        TextFileStream stream(a->Str(1), false);
        ret = SpitAllocInfo(&stream);
    }
    return ret;
}

void MemTracker::DiffDump(TextStream &ts) {
    if (mTimeSlice) {
        ts << "(executable " << TheSystemArgs.front() << ")\n";
        ts << "(data\n";
        int count = 0;
        for (auto it = mHashTable->Begin(); it != nullptr; it = mHashTable->Next(it)) {
            if (mTimeSlice == (*it)->mTimeSlice) {
                count++;
            }
        }
        {
            AllocInfoVec vec(DebugHeapAlloc(count * 4), count * 4);
            for (auto it = mHashTable->Begin(); it != nullptr;
                 it = mHashTable->Next(it)) {
                if (mTimeSlice == (*it)->mTimeSlice) {
                    vec.push_back(*it);
                }
            }
            std::sort(vec.begin(), vec.end(), StackLess);
            std::sort(mFreedInfos.begin(), mFreedInfos.end(), StackLess);
            auto vecIt = vec.begin();
            auto freedIt = mFreedInfos.begin();
            while (vecIt != vec.end() || freedIt != mFreedInfos.end()) {
                int cmp;
                if (vecIt == vec.end()) {
                    cmp = 1;
                } else if (freedIt == mFreedInfos.end()) {
                    cmp = -1;
                } else {
                    cmp = (*vecIt)->StackCompare(*(*freedIt));
                }
                if (cmp < 0) {
                    ColatedPrint(ts, *vecIt++, "alloc");
                } else if (cmp > 0) {
                    ColatedPrint(ts, *freedIt++, "free");
                } else {
                    ++vecIt;
                    ++freedIt;
                }
            }
        }
        ts << ")\n";
    }
    mFreedInfos.delete_and_clear();
    mTimeSlice++;
}

void MemTracker::Report(int minSize, TextStream &stream) {
    HeapReport(stream);
    UpdateStats();
    mHeapTypeStats[mCurStatTable].SortBySize();
    int curNumStats = mHeapTypeStats[mCurStatTable].GetNumStats();
    stream << MakeString(
        "\n  %-30s %2s %5s %10s %10s\n", "TYPE", "Hp", "Num", "SzRequest", "SzActual"
    );
    for (int i = 0; i < curNumStats; i++) {
        BlockStat &curStat = mHeapTypeStats[mCurStatTable].GetBlockStat(i);
        if (curStat.mSizeAct >= minSize) {
            stream << MakeString(
                "  %-30s %2d %5d %10d %10d\n",
                curStat.mName,
                curStat.mHeap,
                curStat.mNumAllocs,
                curStat.mSizeReq,
                curStat.mSizeAct
            );
        }
    }
    mPoolTypeStats[mCurStatTable].SortBySize();
    curNumStats = mPoolTypeStats[mCurStatTable].GetNumStats();
    stream << MakeString(
        "\n  %-30s %5s %10s %10s\n", "POOL TYPE", "Num", "SzRequest", "SzActual"
    );
    for (int i = 0; i < curNumStats; i++) {
        BlockStat &curStat = mPoolTypeStats[mCurStatTable].GetBlockStat(i);
        if (curStat.mSizeAct >= minSize) {
            stream << MakeString(
                "  %-30s %5d %10d %10d\n",
                curStat.mName,
                curStat.mNumAllocs,
                curStat.mSizeReq,
                curStat.mSizeAct
            );
        }
    }
    stream << "Diff from last report:\n";
    DiffTblReport(
        "MALLOC DIFF TYPES",
        mHeapTypeStats[mCurStatTable],
        mHeapTypeStats[1 - mCurStatTable],
        stream
    );
    DiffTblReport(
        "POOL DIFF TYPES",
        mPoolTypeStats[mCurStatTable],
        mPoolTypeStats[1 - mCurStatTable],
        stream
    );
    mCurStatTable = 1 - mCurStatTable;
}

void MemTracker::ReportMemoryAlloc(const char *cc) {
    const char *venue = TheGameData->Venue().Str();
    const char *song = TheGameData->GetSong().Str();
    const char *char1 = nullptr;
    const char *char2 = nullptr;
    HamPlayerData *p1 = TheGameData->Player(0);
    if (p1) {
        char1 = p1->Char().Str();
    }
    HamPlayerData *p2 = TheGameData->Player(1);
    if (p2) {
        char2 = p2->Char().Str();
    }
    char buffer[128];
    Hx_snprintf(
        buffer,
        sizeof(buffer),
        "%s_%s_%s_%s_%s_%s_alloc_info.csv",
        mAllocInfoName,
        cc,
        venue,
        char1,
        char2,
        song
    );
    TextFileStream stream(buffer, false);
    SpitAllocInfo(&stream);
    stream.File().Flush();
}

void MemTracker::ReportMemoryUsage(const char *cc) {
    TextStream *stream = &TheDebug;
    if (mReport) {
        stream = mReport;
    }
    static bool sHeaderPrinted = false;
    if (!sHeaderPrinted) {
        *stream
            << MakeString("Category,heap,free,biggest,lfrags,requested,allocated,peak\n");
        sHeaderPrinted = true;
    }
    int numHeaps = MemNumHeaps() + 1;
    for (int i = 0; i < numHeaps; i++) {
        *stream << MakeString(cc);
        if (i == MemNumHeaps()) {
            int freePhys = _GetFreePhysicalMemory();
            int used = mFreePhysMem - PhysicalUsage();
            if (used < freePhys) {
                used = freePhys;
            }
            *stream << MakeString(",physicalHeap");
            *stream << MakeString(",%d", used);
            *stream << MakeString(",%d", freePhys);
            *stream << MakeString(",0");
        } else {
            int lFrags, rFrags, numFreeBytes, i5, biggestFreeBlock;
            MemFreeBlockStats(i, lFrags, rFrags, numFreeBytes, i5, biggestFreeBlock);
            *stream << MakeString(",%sHeap", MemHeapName(i));
            *stream << MakeString(",%d", numFreeBytes);
            *stream << MakeString(",%d", biggestFreeBlock);
            *stream << MakeString(",%d", lFrags);
        }
        *stream << MakeString(",%d", mHeapStats[i].mTotalReqSize);
        *stream << MakeString(",%d", mHeapStats[i].mTotalActSize);
        *stream << MakeString(",%d\n", mHeapStats[i].mMaxActSize);
    }
}

void MemTracker::ReportMemoryUsageOverview(const char *cc) {
    TextStream *stream = &TheDebug;
    if (mReport) {
        stream = mReport;
    }
    *stream << MakeString(
        "\nCategory,Mode,MainPeak,MainAlloc,MainLargest,CharPeak,CharAlloc,CharLargest,PhysPeak,PhysAlloc,PhysLargest\n"
    );
    int numHeaps = MemNumHeaps() + 1;
    *stream << "overview," << cc;
    for (int i = 0; i < numHeaps; i++) {
        int used;
        int i7c;
        int w, y, z;
        if (i == MemNumHeaps()) {
            int freePhys = _GetFreePhysicalMemory();
            used = mFreePhysMem - PhysicalUsage();
            if (used < freePhys) {
                used = freePhys;
            }
            i7c = 0;
        } else {
            MemFreeBlockStats(i, i7c, w, used, y, z);
        }
        *stream << MakeString(",%d", mHeapStats[i].mMaxActSize);
        *stream << MakeString(",%d", mHeapStats[i].mTotalActSize);
        *stream << MakeString(",%d", z);
    }
}

int MemTracker::SpitAllocInfo(TextStream *stream) {
    int ret = 1;
    if (gMemTracker && gMemTracker->mHashTable) {
        MILO_LOG("----------------BEGIN MemTracker::SpitAllocInfo\n");
        for (auto it = gMemTracker->mHashTable->Begin(); it != nullptr;
             it = gMemTracker->mHashTable->Next(it)) {
            (*it)->Print(*stream);
        }
        MILO_LOG("----------------END MemTracker::SpitAllocInfo\n");
        ret = 0;
    }
    return ret;
}

int MemTracker::SpitAllocInfo(FILE *file) {
    int ret = 1;
    if (gMemTracker && gMemTracker->mHashTable) {
        MILO_LOG("----------------BEGIN MemTracker::SpitAllocInfo\n");
        for (auto it = gMemTracker->mHashTable->Begin(); it != nullptr;
             it = gMemTracker->mHashTable->Next(it)) {
            (*it)->PrintForReport(file);
        }
        MILO_LOG("----------------END MemTracker::SpitAllocInfo\n");
        ret = 0;
    }
    return ret;
}
