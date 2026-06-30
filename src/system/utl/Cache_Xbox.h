#pragma once
#include "obj/Object.h"
#include "os/ThreadCall.h"
#include "utl/Cache.h"
#include "utl/Str.h"
#include <cstring>
#include "xdk/XAPILIB.h"

class CacheIDXbox : public CacheID {
public:
    CacheIDXbox();
    virtual ~CacheIDXbox() {}
    virtual const char *GetCachePath(const char *);
    virtual const char *GetCacheSearchPath(const char *);
    virtual unsigned int GetDeviceID() const { return mContentData.DeviceID; }

    const char *Name() const { return mStrCacheName.c_str(); }
    XCONTENT_DATA *ContentData() { return &mContentData; }
    void SetName(const String &name) { mStrCacheName = name; }

private:
    String mStrCacheName; // 0x4
    XCONTENT_DATA mContentData; // 0xc
};

class CacheXbox : public Cache, public ThreadCallback {
public:
    CacheXbox(const CacheIDXbox &);
    virtual ~CacheXbox() {}
    virtual const char *GetCacheName() { return mCacheID.Name(); }
    virtual void Poll() {}
    virtual bool IsConnectedSync();
    virtual bool GetFileSizeAsync(const char *, unsigned int *, Hmx::Object *);
    virtual bool ReadAsync(const char *, void *, unsigned int, Hmx::Object *);
    virtual bool DeleteAsync(const char *, Hmx::Object *);
    virtual bool GetFreeSpaceSync(u64 *);
    virtual bool
    GetDirectoryAsync(const char *, std::vector<CacheDirEntry> *, Hmx::Object *);
    virtual bool DeleteSync(const char *);
    virtual bool WriteAsync(const char *, void *, unsigned int, Hmx::Object *);

protected:
    virtual int ThreadStart();
    virtual void ThreadDone(int);

    int ThreadGetFileSize();
    int ThreadRead();
    int ThreadWrite();
    bool DeleteParentDirs(String);
    int ThreadDelete();
    int ThreadGetDir(String, String);

    CacheIDXbox mCacheID; // 0x10
    String mThreadStr; // 0x150
    void *mData; // 0x158
    unsigned int mSize; // 0x15c
    std::vector<CacheDirEntry> *mCacheDirList; // 0x160
    Hmx::Object *mCallbackObj; // 0x164
};
