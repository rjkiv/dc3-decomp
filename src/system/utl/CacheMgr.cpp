#include "utl/CacheMgr.h"
#include "Cache.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "stl/_vector.h"
#include "utl/Cache.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

CacheMgr *TheCacheMgr;

CacheMgr::CacheMgr() : unk10(), unk14(kCache_NoError) {}

CacheMgr::~CacheMgr() {}

bool CacheMgr::SearchAsync(char const *, CacheID **) {
    MILO_FAIL("CacheMgr::SearchAsync() not supported by this platform.\n");
    unk14 = kCache_ErrorUnknown;
    return false;
}

bool CacheMgr::ShowUserSelectUIAsync(
    LocalUser *, unsigned long long, char const *, char const *, CacheID **
) {
    MILO_FAIL("CacheMgr::ShowUserSelectUIAsync() not supported by this platform.\n");
    unk14 = kCache_ErrorUnknown;
    return false;
}

bool CacheMgr::CreateCacheIDFromDeviceID(
    unsigned int, const char *, const char *, CacheID **
) {
    MILO_FAIL("CacheMgr::SetDeviceID() not supported by this platform.\n");
    unk14 = kCache_ErrorUnknown;
    return false;
}

bool CacheMgr::CreateCacheID(
    const char *, const char *, const char *, const char *, const char *, int, CacheID **
) {
    MILO_FAIL("CacheMgr::CreateCacheID() not supported by this platform.\n");
    unk14 = kCache_ErrorUnknown;
    return false;
}

CacheMgr *CacheMgr::CreateCacheMgr() { return 0; }

CacheID *CacheMgr::GetCacheID(Symbol s) { return 0; }

void CacheMgr::RemoveCacheID(CacheID *id) {
    FOREACH (it, unk4) {
        CacheMgr::CacheIDStoreEntry &entry = (*it);
        if (entry.id == id) {
            unk4.erase(it);
        } else
            it++;
    }
}

void CacheMgr::AddCacheID(CacheID *id, Symbol s) {
    std::vector<CacheMgr::CacheIDStoreEntry>::iterator it = unk4.begin();
    while (it != unk4.end()) {
        CacheMgr::CacheIDStoreEntry &entry = (*it);
        if (entry.symbol == s) {
            entry.id = id;
            return;
        }
        it++;
    }
    MILO_ASSERT(it == unk4.end(), 0x8a);
    CacheIDStoreEntry e(s, id);
    unk4.push_back(e);
}

bool CacheMgr::IsDone() { return unk10 == 0; }

void CacheMgr::SetLastResult(CacheResult c) { unk14 = c; }

void CacheMgrTerminate() {
    delete TheCacheMgr;
    TheCacheMgr = nullptr;
}

void CacheMgrInit() {
    MILO_ASSERT(TheCacheMgr == NULL, 0x12);
    MILO_ASSERT(TheCacheMgr != NULL, 0x14);
}
