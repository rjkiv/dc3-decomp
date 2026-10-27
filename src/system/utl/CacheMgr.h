#pragma once
#include "obj/Object.h"
#include "os/User.h"
#include "utl/Cache.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include <vector>

class CacheMgr {
public:
    enum OpType {
        kOpNone = 0,
        kOpSearch = 1,
        kOpChoose = 2,
        kOpMount = 3,
        kOpUnmount = 4,
        kOpDelete = 5,
    };

    struct CacheIDStoreEntry {
        CacheIDStoreEntry(Symbol s, CacheID *cid) : mName(s), mCacheID(cid) {}
        Symbol mName; // 0x0
        CacheID *mCacheID; // 0x4
    };

    virtual ~CacheMgr();
    virtual void Poll() = 0;
    virtual bool SearchAsync(const char *strCacheName, CacheID **ppCacheID);
    virtual bool ShowUserSelectUIAsync(
        LocalUser *pUser,
        u64 sizeMin,
        const char *strCacheName,
        const char *strCacheDescription,
        CacheID **ppCacheID
    );
    virtual bool CreateCacheIDFromDeviceID(
        unsigned int deviceID,
        const char *strCacheName,
        const char *utf8CacheDescription,
        CacheID **ppCacheID
    );
    virtual bool CreateCacheID(
        const char *strCacheName,
        const char *strIconFilename,
        const char *strTitleID,
        const char *strTitleName,
        const char *strDataVersion,
        int minSizeKB,
        CacheID **ppCacheID
    );
    virtual bool
    MountAsync(CacheID *pCacheID, Cache **ppCache, Hmx::Object *pCallbackObj) = 0;
    virtual bool UnmountAsync(Cache **ppCache, Hmx::Object *pCallbackObj) = 0;
    virtual bool DeleteAsync(CacheID *pCacheID) = 0;

    CacheID *GetCacheID(Symbol symName);
    void RemoveCacheID(CacheID *pCacheID);
    void AddCacheID(CacheID *pCacheID, Symbol symName);
    bool IsDone();
    CacheResult GetLastResult();

    static CacheMgr *CreateCacheMgr();

    MEM_OVERLOAD(CacheMgr, 0x24);

protected:
    CacheMgr();
    OpType GetOp();
    void SetOp(OpType);
    void SetLastResult(CacheResult);

    std::vector<CacheIDStoreEntry> mCacheIDStore; // 0x4
private:
    OpType mOpCur; // 0x10
    CacheResult mLastResult; // 0x14
};

void CacheMgrTerminate();
void CacheMgrInit();
bool IsDeviceConnected(DWORD deviceID);

extern CacheMgr *TheCacheMgr;
