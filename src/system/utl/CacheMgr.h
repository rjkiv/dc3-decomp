#pragma once
#include "os/User.h"
#include "stl/_vector.h"
#include "utl/Cache.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

class CacheMgr {
public:
    enum OpType {
    };

    struct CacheIDStoreEntry {
        CacheIDStoreEntry(Symbol s, CacheID *cid) : symbol(s), id(cid) {}
        Symbol symbol;
        CacheID *id;
    };

    virtual ~CacheMgr();
    virtual bool SearchAsync(char const *, CacheID **);
    virtual bool ShowUserSelectUIAsync(
        LocalUser *, unsigned long long, char const *, char const *, CacheID **
    );
    virtual bool
    CreateCacheIDFromDeviceID(unsigned int, const char *, const char *, CacheID **);
    virtual bool CreateCacheID(
        const char *,
        const char *,
        const char *,
        const char *,
        const char *,
        int,
        CacheID **
    );

    static CacheMgr *CreateCacheMgr();
    CacheID *GetCacheID(Symbol);
    void RemoveCacheID(CacheID *);
    void AddCacheID(CacheID *, Symbol);
    bool IsDone();

    MEM_OVERLOAD(CacheMgr, 0x24);

    std::vector<CacheMgr::CacheIDStoreEntry> unk4;
    CacheMgr::OpType unk10;
    CacheResult unk14;
    int unk18;

protected:
    CacheMgr();
    void SetLastResult(CacheResult);
};

void CacheMgrTerminate();
void CacheMgrInit();

extern CacheMgr *TheCacheMgr;
