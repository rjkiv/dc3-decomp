#include "utl/AllocInfo.h"
#include "utl/Pool.h"
#include "os/Debug.h"
#include "trie.h"
#include "utl/TextStream.h"
#include "xdk/XBDM.h"

Trie *s_pTrie;
bool AllocInfo::bPrintCsv;

Pool &GetPool() {
    static void *sMem;
    static Pool sPool(4, sMem, 4);
    return sPool;
}

AllocInfo::AllocInfo(
    int requestedSize,
    int actualSize,
    const char *type,
    void *mem,
    signed char heap,
    bool pooled,
    unsigned char strat,
    const char *file,
    int line,
    String &str1,
    String &str2
)
    : mReqSize(requestedSize), mActSize(actualSize), mType(type), mMem(mem), mHeap(heap),
      mPooled(pooled), mStrat(strat), mFile(file), mLine(line),
      unk1d(s_pTrie->store(str1.c_str())), unk21(s_pTrie->store(str2.c_str())) {
    FillStackTrace();
}

AllocInfo::~AllocInfo() {
    MILO_ASSERT(s_pTrie, 0x6D);
    s_pTrie->remove(unk1d);
    s_pTrie->remove(unk21);
}

void *AllocInfo::operator new(unsigned int size) {
    MILO_ASSERT(size == sizeof(AllocInfo), 0x32);
    void *mem = GetPool().Alloc();
    MILO_ASSERT(mem, 0x36);
    return mem;
}

void AllocInfo::operator delete(void *mem) { GetPool().Free(mem); }

void AllocInfo::SetPoolMemory(void *mem, int i2) { GetPool() = Pool(0x65, mem, i2); }

void AllocInfo::Validate() const { MILO_ASSERT(mPooled <= 1, 0xA5); }

void AllocInfo::PrintCsv(TextStream &ts) const {
    ts << MakeString("addr, 0x%lX, %s, bytes, %d ", (unsigned long)mMem, mType, mReqSize);
    MILO_ASSERT(s_pTrie, 0xC6);
    char buf1d[0x80];
    char buf21[0x80];
    ts << ", actual, " << mActSize << ", heap, " << mHeap << ", " << mFile << ", "
       << mLine << ", " << s_pTrie->get(unk1d, buf1d, 0x80) << ", "
       << s_pTrie->get(unk21, buf21, 0x80);
    if (mPooled) {
        ts << ", pooled";
    }
}

void AllocInfo::Print(TextStream &ts) const {
    if (bPrintCsv)
        PrintCsv(ts);
    else {
        ts << MakeString(
            "(addr 0x%lX ) (type \"%s\") (bytes %d) ", (unsigned long)mMem, mType, mReqSize
        );
        if (mPooled)
            ts << "(pooled) ";
        ts << "(actual " << mActSize << ") (heap_number " << mHeap << ") (location "
           << mFile << " " << mLine << ") ";
        ts << "(stack ";
        for (int i = 0; mStackTrace[i] != 0 && i < 16; i++) {
            ts << mStackTrace[i] << " ";
        }
        ts << ") ";
    }
}

TextStream &operator<<(TextStream &ts, const AllocInfo &info) {
    info.Print(ts);
    return ts;
}

int AllocInfo::Compare(const AllocInfo &info) const {
    int cmp = strcmp(mType, info.mType);
    if (cmp) {
        return cmp;
    } else if (mReqSize < info.mReqSize) {
        return -1;
    } else
        return mReqSize <= info.mReqSize;
}

void AllocInfo::FillStackTrace() {
    int stack[20];
    DmCaptureStackBackTrace(20, stack);
    for (int i = 0; i < 16; i++) {
        mStackTrace[i] = stack[i];
        if (stack[i] == 0U)
            break;
    }
    mStackTrace[15] = 0;
}

void AllocInfoInit() {
    void *dst;
    if (s_pTrie == nullptr) {
        // 0x220008
        dst = MemAlloc(sizeof(Trie), __FILE__, 0x28, "Trie, 0");
        if (dst == nullptr) {
            s_pTrie = nullptr;
        } else {
            memset(dst, 0, sizeof(Trie));
            // some trie member bool being set to true here
            s_pTrie = (Trie *)dst;
        }
    }
}

int AllocInfo::StackCompare(const AllocInfo &other) const {
    int result = Compare(other);
    if (result != 0) {
        return result;
    }
    for (int i = 0; i < 16; i++) {
        if (mStackTrace[i] < other.mStackTrace[i]) {
            return -1;
        }
        if (mStackTrace[i] > other.mStackTrace[i]) {
            return 1;
        }
        if (mStackTrace[i] == 0 && other.mStackTrace[i] == 0) {
            break;
        }
    }
    return 0;
}