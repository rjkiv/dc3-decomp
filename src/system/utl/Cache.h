#pragma once

#include "utl/MemMgr.h"
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

struct CacheDirEntry {};

class CacheID {
public:
    virtual ~CacheID() = 0;
    virtual const char *GetCachePath(const char *) = 0;
    virtual const char *GetCacheSearchPath(const char *) = 0;
    virtual unsigned int GetDeviceID() const;

    CacheID() {}
};

class Cache {
public:
    virtual ~Cache();

    Cache();
    bool IsDone();

    MEM_OVERLOAD(Cache, 0x56);

    OpType mOpCur; // 0x4
    CacheResult mLastResult; // 0x8
};
