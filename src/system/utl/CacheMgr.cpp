#include "utl/CacheMgr.h"
#include "Cache.h"
#include "CacheMgr_Xbox.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Cache.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

CacheMgr *TheCacheMgr;

CacheMgr::CacheMgr() : mOpCur(kOpNone), mLastResult(kCache_NoError) {}
CacheMgr::~CacheMgr() {}

bool CacheMgr::SearchAsync(char const *, CacheID **) {
    MILO_FAIL("CacheMgr::SearchAsync() not supported by this platform.\n");
    mLastResult = kCache_ErrorUnknown;
    return false;
}

bool CacheMgr::ShowUserSelectUIAsync(
    LocalUser *, u64, char const *, char const *, CacheID **
) {
    MILO_FAIL("CacheMgr::ShowUserSelectUIAsync() not supported by this platform.\n");
    mLastResult = kCache_ErrorUnknown;
    return false;
}

bool CacheMgr::CreateCacheIDFromDeviceID(
    unsigned int, const char *, const char *, CacheID **
) {
    MILO_FAIL("CacheMgr::SetDeviceID() not supported by this platform.\n");
    mLastResult = kCache_ErrorUnknown;
    return false;
}

bool CacheMgr::CreateCacheID(
    const char *, const char *, const char *, const char *, const char *, int, CacheID **
) {
    MILO_FAIL("CacheMgr::CreateCacheID() not supported by this platform.\n");
    mLastResult = kCache_ErrorUnknown;
    return false;
}

CacheID *CacheMgr::GetCacheID(Symbol s) {
    FOREACH (it, mCacheIDStore) {
        CacheIDStoreEntry cur = *it;
        if (cur.mName == s)
            return cur.mCacheID;
    }
    return nullptr;
}

void CacheMgr::RemoveCacheID(CacheID *id) {
    for (auto it = mCacheIDStore.begin(); it != mCacheIDStore.end();) {
        if (it->mCacheID == id) {
            it = mCacheIDStore.erase(it);
        } else
            ++it;
    }
}

void CacheMgr::AddCacheID(CacheID *id, Symbol s) {
    auto it = mCacheIDStore.begin();
    for (; it != mCacheIDStore.end(); ++it) {
        if (it->mName == s) {
            it->mCacheID = id;
            return;
        }
    }
    MILO_ASSERT(it == mCacheIDStore.end(), 0x8A);
    mCacheIDStore.push_back(CacheIDStoreEntry(s, id));
}

bool CacheMgr::IsDone() { return mOpCur == kOpNone; }
void CacheMgr::SetLastResult(CacheResult c) { mLastResult = c; }
void CacheMgr::SetOp(OpType t) { mOpCur = t; }
CacheMgr::OpType CacheMgr::GetOp() { return mOpCur; }
CacheMgr *CacheMgr::CreateCacheMgr() { return new CacheMgrXbox(); }

void CacheMgrInit() {
    MILO_ASSERT(TheCacheMgr == NULL, 0x12);
    TheCacheMgr = CacheMgr::CreateCacheMgr();
    MILO_ASSERT(TheCacheMgr != NULL, 0x14);
}

void CacheMgrTerminate() { RELEASE(TheCacheMgr); }
