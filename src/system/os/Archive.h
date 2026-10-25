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
    int GetHeapSize() const { return mFree - mHeap; }

    //   public: ArkHash(int32_t);
    //   public: void Write(BinStream&) const;
    //   public: void Read(BinStream&, int32_t);
    //   public: int32_t GetHeapSize() const;
    //   public: int32_t GetNumStrings() const;

private:
    char *mHeap; // 0x0
    char *mHeapEnd; // 0x4
    char *mFree; // 0x8
    char **mTable; // 0xc
    int mTableSize; // 0x10
};

class FileEntry {
public:
    FileEntry() {}
    FileEntry(int name, int path, u64 offset, int size, int ucSize)
        : mOffset(offset), mHashedName(name), mHashedPath(path), mSize(size),
          mUCSize(ucSize) {}

    bool operator<(const FileEntry &e) const {
        return mHashedPath < e.mHashedPath
            || (mHashedPath == e.mHashedPath && mHashedName < e.mHashedName);
    }

    u64 mOffset; // 0x0
    int mHashedName; // 0x8
    int mHashedPath; // 0xc
    int mSize; // 0x10
    int mUCSize; // 0x14
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
    bool IsPatched() const { return mIsPatched; }
    int GetNumArkfiles() const { return mNumArkfiles; }
    int GetHeapSize() const { return mHashTable.GetHeapSize(); }

    static bool DebugArkOrder();

    MEM_OVERLOAD(Archive, 100);

    // clang-format off

    //  public: void AddFile(const String&, int32_t, int32_t);
    //  public: void Write(bool);
    //  public: void Print(TextStream&);
    //  public: void FindFilesInRange(uint64_t, uint64_t, stlpmtx_std::list<String,stlpmtx_std::StlNodeAlloc<String> >&);
    //  public: void Verify();
    //  public: bool IsValidBlock(int32_t, int32_t);
    //  public: bool IsArkSplitEntry(const FileEntry&);

    //  public: const char* GetArkfileBaseName(int32_t) const;
    //  public: int32_t GetHeapSize() const;
    //  public: int32_t GetNumStrings() const;
    //  public: void SetBasename(const char*);
    // clang-format on

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
