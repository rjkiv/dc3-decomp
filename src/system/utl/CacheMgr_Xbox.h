#pragma once
#include "obj/Object.h"
#include "utl/Cache.h"
#include "utl/CacheMgr.h"
#include "utl/Cache_Xbox.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"

// size 0x18c
class CacheMgrXbox : public CacheMgr {
public:
    CacheMgrXbox();
    virtual ~CacheMgrXbox();
    virtual void Poll();
    virtual bool SearchAsync(const char *, CacheID **);
    virtual bool
    ShowUserSelectUIAsync(LocalUser *, u64, const char *, const char *, CacheID **);
    virtual bool
    CreateCacheIDFromDeviceID(unsigned int, const char *, const char *, CacheID **);
    virtual bool MountAsync(CacheID *, Cache **, Hmx::Object *);
    virtual bool UnmountAsync(Cache **, Hmx::Object *);
    virtual bool DeleteAsync(CacheID *);

private:
    void PollSearch();
    void PollChoose();
    void PollMount();
    void PollUnmount();
    void PollDelete();

    void CreateCacheIDForChosenDevice();
    void EndSearch(CacheResult);

    HANDLE mFile; // 0x18
    XOVERLAPPED mOverlapped; // 0x1c
    CacheID **mppCacheID; // 0x38
    Cache **mppCache; // 0x3c
    Hmx::Object *mCallback; // 0x40
    CacheIDXbox *mCacheIDXbox; // 0x44
    XCONTENT_DATA mContentData; // 0x48
    String mStrCacheName; // 0x17c
    String mUTF8CacheDescription; // 0x184
};
