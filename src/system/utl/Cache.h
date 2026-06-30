#pragma once
#include "obj/Object.h"
#include "os/DateTime.h"
#include "types.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

enum OpType {
    kOpNone = 0,
    kOpDirectory = 1,
    kOpFileSize = 2,
    kOpRead = 3,
    kOpWrite = 4,
    kOpDelete = 5,
};
enum CacheResult {
    kCache_NoError = 0,
    kCache_ErrorBusy = 1,
    kCache_ErrorBadParam = 2,
    kCache_ErrorUnsupported = 3,
    kCache_ErrorUserCancel = 4,
    kCache_ErrorNoStorageDevice = 5,
    kCache_ErrorCacheNotFound = 6,
    kCache_ErrorCorrupt = 7,
    kCache_ErrorStorageDeviceMissing = 8,
    kCache_ErrorNotEnoughSpace = 9,
    kCache_Error360GuideAlreadyOut = 10,
    kCache_ErrorUnknown = -1,
};

enum CacheResourceResult {
    kCacheUnnecessary = 0,
    kCacheUnknownExtension = 1,
    kCacheMissingFile = 2,
    kCacheFailed = 3,
    kCacheSuccessful = -1
};

class CacheDirEntry {
public:
    String mFullFileName; // 0x0
    DateTime mLastWriteTime; // 0x8
    DWORD mFileSize; // 0x10
};

class CacheID {
public:
    CacheID() {}
    virtual ~CacheID() {}
    virtual const char *GetCachePath(const char *) = 0;
    virtual const char *GetCacheSearchPath(const char *) = 0;
    virtual unsigned int GetDeviceID() const;

    MEM_OVERLOAD(CacheID, 0x2F);
};

class Cache {
public:
    Cache();
    virtual ~Cache();
    virtual const char *GetCacheName() = 0;
    virtual void Poll() = 0;
    virtual bool IsConnectedSync() = 0;
    virtual bool GetFreeSpaceSync(u64 *) = 0;
    virtual bool DeleteSync(const char *) = 0;
    virtual bool
    GetDirectoryAsync(const char *, std::vector<CacheDirEntry> *, Hmx::Object *) = 0;
    virtual bool GetFileSizeAsync(const char *, unsigned int *, Hmx::Object *) = 0;
    virtual bool ReadAsync(const char *, void *, unsigned int, Hmx::Object *) = 0;
    virtual bool WriteAsync(const char *, void *, unsigned int, Hmx::Object *) = 0;
    virtual bool DeleteAsync(const char *, Hmx::Object *) = 0;

    bool IsDone();
    CacheResult GetLastResult();
    MEM_OVERLOAD(Cache, 0x56);

protected:
    OpType mOpCur; // 0x4
    CacheResult mLastResult; // 0x8
};
