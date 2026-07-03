#pragma once
#include "obj/Object.h"
#include "os/File.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"

class FileCacheHelper {
public:
    virtual ~FileCacheHelper() {}
    virtual const char *CacheFile(const char *) = 0;
};

class FileCacheEntry;

class FileCacheFile : public File {
public:
    FileCacheFile(FileCacheEntry *entry);
    // File
    virtual ~FileCacheFile();
    virtual int Read(void *, int);
    virtual bool ReadAsync(void *, int);
    virtual int Write(const void *, int) {
        MILO_FAIL("not implemented");
        return 0;
    }
    virtual int Seek(int, int);
    virtual int Tell() { return mPos; }
    virtual void Flush() {}
    virtual bool Eof();
    virtual bool Fail();
    virtual int Size();
    virtual int UncompressedSize() { return 0; }
    virtual bool ReadDone(int &);
    virtual bool GetFileHandle(void *&) { return false; }

    POOL_OVERLOAD(FileCacheFile, 0x2D);

private:
    FileCacheEntry *mParent; // 0x4
    int mBytesRead; // 0x8
    void *mData; // 0xc
    int mPos; // 0x10
};

class FileCache {
public:
    FileCache(int, LoaderPos, bool, bool);
    ~FileCache();

    bool DoneCaching();
    bool FileCached(char const *);
    void StartSet(int);
    void Clear();
    void PollUntilLoaded();
    void Add(FilePath const &, int, FilePath const &);
    void Add(FilePath const &, char *, int);
    void EndSet();
    void SetSize(int);

    static void Init();
    static void Terminate();
    static void PollAll();
    static File *GetFileAll(char const *);
    static void RegisterResourceCacheHelper(class FileCacheHelper *);
    static void RegisterWavCacheHelper(class FileCacheHelper *);

    MEM_OVERLOAD(FileCache, 0x21);

protected:
    int mMaxSize; // 0x0
    bool mTryClear; // 0x4
    std::vector<FileCacheEntry *> mEntries; // 0x8
    LoaderPos unk14;
    bool unk18;
    bool unk19;

    static FileCacheHelper *sResourceCacheHelper;
    static FileCacheHelper *sWavCacheHelper;

    File *GetFile(char const *);
    int CurSize() const;
    void DumpOverSize(int);
    void Poll();
};
