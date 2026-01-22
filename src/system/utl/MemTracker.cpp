#include "utl/MemTracker.h"
#include "AllocInfo.h"
#include "MemMgr.h"
#include "MemTrack.h"
#include "Memory.h"
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

void DiffTblReport(const char *, BlockStatTable &, BlockStatTable &, TextStream &);

MemTracker::MemTracker(int x, int y)
    : mHashMem(nullptr), mHashTable(nullptr), mTimeSlice(0), mCurStatTable(0),
      mFreedInfos(y), mLog(0), mReport(0), mHeap(x) {
    mHashMem = DebugHeapAlloc(y * 8);
    MILO_ASSERT(mHashMem, 0x4E);
    mHashTable = new KeylessHash<void *, AllocInfo *>(
        x * 2, (AllocInfo *)0, (AllocInfo *)-1, (AllocInfo **)mHashMem
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
        String str1;
        String str2;
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
            str1,
            str2
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
                             << str1.c_str() << ":" << str2.c_str() << "\n";
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
    *mLog = ts;
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
    mPoolTable[mCurStatTable].Clear();
    mMemTable[mCurStatTable].Clear();
    for (auto it = mHashTable->Begin(); it != nullptr; it = mHashTable->Next(it)) {
        AllocInfo *info = *it;
        if (info->mPooled) {
            mPoolTable[mCurStatTable].Update(
                info->mType, info->mHeap, info->mReqSize, info->mActSize
            );
        } else {
            mMemTable[mCurStatTable].Update(
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
            AllocInfo *info = *it;
            if (mTimeSlice == info->mTimeSlice) {
                count++;
            }
        }
        AllocInfoVec vec(count);
        for (auto it = mHashTable->Begin(); it != nullptr; it = mHashTable->Next(it)) {
            AllocInfo *info = *it;
            if (mTimeSlice == info->mTimeSlice) {
                vec.push_back(info);
            }
        }
        std::sort(vec.begin(), vec.end(), StackLess);
        std::sort(mFreedInfos.begin(), mFreedInfos.end(), StackLess);
        // iterate across both AllocInfoVecs here
    }
    mFreedInfos.delete_and_clear();
    mTimeSlice++;
}
