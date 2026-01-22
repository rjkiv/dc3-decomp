#pragma once
#include "MemTrack.h"
#include "os/Debug.h"
#include "utl/Str.h"
#include "utl/trie.h"
#include "utl/TextStream.h"

// size 0x65
#pragma pack(push, 1)
class AllocInfo {
public:
    AllocInfo(
        int requestedSize,
        int actualSize,
        const char *type,
        void *mem,
        signed char heap,
        bool pooled,
        unsigned char strat,
        const char *file,
        int line,
        String &,
        String &
    );
    ~AllocInfo();

    int Compare(const AllocInfo &) const;
    void FillStackTrace();
    void Validate() const;

    void PrintCsv(TextStream &) const;
    void PrintForReport(TextStream &) const;
    void Print(TextStream &) const;
    int StackCompare(const AllocInfo &) const;

    static bool bPrintCsv;
    static void SetPoolMemory(void *, int);
    static void *operator new(unsigned int);
    static void operator delete(void *);

    int mReqSize; // 0x0
    int mActSize; // 0x4
    const char *mType; // 0x8
    void *mMem; // 0xc
    signed char mHeap; // 0x10
    bool mPooled; // 0x11
    short mTimeSlice; // 0x12
    unsigned char mStrat; // 0x14
    const char *mFile; // 0x15
    int mLine; // 0x19
    unsigned int unk1d; // 0x1d
    unsigned int unk21; // 0x21
    int mStackTrace[0x10]; // 0x25
};
#pragma pack(pop)

TextStream &operator<<(TextStream &, const AllocInfo &);

class AllocInfoVec {
public:
    AllocInfoVec() : mStart(0), mEnd(0), mEndOfStorage(0) {}
    __forceinline AllocInfoVec(int size)
        : mStart((AllocInfo **)DebugHeapAlloc(size * 4)), mEnd(mStart),
          mEndOfStorage(mStart + size) {}
    ~AllocInfoVec() { DebugHeapFree(mStart); }

    AllocInfo **begin() { return mStart; }
    AllocInfo **end() { return mEnd; }

    void push_back(AllocInfo *info) {
        MILO_ASSERT(mEnd < mEndOfStorage, 0x61);
        *mEnd++ = info;
    }

    void delete_and_clear() {
        for (auto it = mStart; it != mEnd; ++it) {
            AllocInfo *info = *it;
            if (info) {
                delete info;
            }
        }
        mEnd = mStart;
    }

private:
    AllocInfo **mStart; // 0x0
    AllocInfo **mEnd; // 0x4
    AllocInfo **mEndOfStorage; // 0x8
};

void AllocInfoInit();