#pragma once
#include "obj/Object.h"
#include "utl/Cache.h"
#include <cstring>

class CacheIDXbox : public CacheID {
public:
    virtual ~CacheIDXbox();
    virtual char const *GetCachePath(char const *);
    virtual char const *GetCacheSearchPath(char const *);

    CacheIDXbox();
    CacheIDXbox(CacheIDXbox const &);

    String mStrCacheName;
    char unkc[308];
};

class CacheXbox : public Cache {
public:
    virtual ~CacheXbox();
    virtual bool IsConnectedSync();
    virtual bool GetFileSizeAsync(char const *, unsigned int *, Hmx::Object *);
    virtual bool ReadAsync(char const *, void *, unsigned int, Hmx::Object *);
    virtual bool DeleteAsync(char const *, Hmx::Object *);
    virtual bool GetFreeSpaceSync(unsigned long long *);
    virtual bool
    GetDirectoryAsync(char const *, std::vector<CacheDirEntry> *, Hmx::Object *);
    virtual bool DeleteSync(char const *);
    virtual bool WriteAsync(char const *, void *, unsigned int, Hmx::Object *);

    CacheXbox(CacheIDXbox const &);

    CacheIDXbox unk10;
    String unk150;
    int unk158;
    int unk15c;
    int unk160;
    int unk164;

protected:
    virtual void ThreadDone(int);
    virtual int ThreadStart();

    int ThreadGetFileSize();
    int ThreadRead();
    int ThreadWrite();
    bool DeleteParentDirs(String);
    int ThreadDelete();
    int ThreadGetDir(String, String);
};
