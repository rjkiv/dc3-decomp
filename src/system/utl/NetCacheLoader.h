#pragma once
#include "os/FileCache.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/NetCacheMgr.h"
#include "utl/NetLoader.h"
#include "utl/Str.h"

class NetCacheLoader {
public:
    enum State {
        // loaded = 3
        // failed = 4
        kS_Nil = -1
    };

    NetCacheLoader(FileCache *, const String &);
    ~NetCacheLoader();
    bool IsLoaded() const;
    bool HasFailed() const;
    bool IsSafeToDelete() const;
    int GetSize();
    char *GetBuffer();
    const char *GetRemotePath() const;
    enum NetCacheMgrFailType GetFailType() const;
    void PollLoading() { Poll(); }

    MEM_OVERLOAD(NetCacheLoader, 0x1C);

    State mState; // 0x0
    FileCache *mCache; // 0x4
    String mRemotePath; // 0x8
    FileLoader *mFileLoader; // 0x10
    char *mFileLoaderBuffer; // 0x14
    NetLoader *mNetLoader; // 0x18
    char *mNetLoaderBuffer; // 0x1c
    int unk20; // 0x20

protected:
    void SetState(NetCacheLoader::State);
    void WriteToCache();
    void Poll();
};
