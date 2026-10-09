#pragma once
#include "utl/HxGuid.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "types.h"
#include "os/Debug.h"
#include <vector>

class ArkHash {
public:
    ArkHash() : mHeap(0), mHeapEnd(0), mFree(0), mTable(0), mTableSize(0) {}
    ~ArkHash();
    int GetHashValue(const char *) const;
    void Read(BinStream &, int);
    int AddString(const char *);
    const char *operator[](int idx) const;

    char *mHeap; // 0x0
    char *mHeapEnd; // 0x4
    char *mFree; // 0x8
    char **mTable; // 0xc
    int mTableSize; // 0x10
};

class FileEntry {
    friend BinStream &operator>>(BinStream &, FileEntry &);
    friend class Archive;

public:
    int HashedPath() const { return mHashedPath; }
    int Size() const { return mSize; }
    int HashedName() const { return mHashedName; }
    bool operator<(const FileEntry &e) const {
        return mHashedPath < e.mHashedPath
            || (mHashedPath == e.mHashedPath && mHashedName < e.mHashedName);
    }

private:
    u64 mOffset;
    int mHashedName;
    int mHashedPath;
    int mSize;
    int mUCSize;
};

const int preinitArk = 1;

class Archive {
public:
    enum Mode {
        kRead = 0,
        kWrite = 1,
    };
    Archive(const char *name, int heap_headroom);
    ~Archive();
    bool GetFileInfo(
        const char *file,
        int &arkfileNum,
        unsigned long long &byteOffset,
        int &fileSize,
        int &fileUCSize
    );
    void Enumerate(
        const char *dir,
        void (*cb)(const char *, const char *),
        bool recurse,
        const char *pattern
    );
    const char *GetArkfileName(int filenum) const;
    void GetGuid(HxGuid &) const;
    bool HasArchivePermission(int) const;
    void SetArchivePermission(int, const int *);
    int GetArkfileCachePriority(int arkfileNum) const;
    int GetArkfileNumBlocks(int filenum) const;
    void SetLocationHardDrive();
    void Merge(Archive &shadow);
    bool Patched() const { return mIsPatched; }
    int NumArkFiles() const { return mNumArkfiles; }

    static bool DebugArkOrder();

    MEM_OVERLOAD(Archive, 100);

private:
    void Read(int heap_headroom);

    int mNumArkfiles; // 0x0
    std::vector<unsigned int> mArkfileSizes; // 0x4
    std::vector<String> mArkfileNames; // 0x10
    std::vector<int> mArkfileCachePriority; // 0x1c
    std::vector<FileEntry> mFileEntries; // 0x28
    ArkHash mHashTable; // 0x34
    String mBasename; // 0x48
    Mode mMode; // 0x50
    unsigned int mMaxArkfileSize; // 0x54
    bool mIsPatched; // 0x58
    HxGuid mGuid; // 0x5c
    const int *mArkfilePerms; // 0x6c
    int mArkfilePermsCount; // 0x70
};

extern Archive *TheArchive;
void ArchiveInit();

extern const int kArkBlockSize;
