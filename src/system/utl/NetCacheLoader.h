#pragma once

#include "utl/Loader.h"
#include "utl/Str.h"
class NetCacheLoader {
public:
    enum State { // copied from NetCacheMgr, probably not the same
        kS_Load,
        kS_Ready,
        kS_UnloadWaitForWrite,
        kS_UnloadUnmount,
        kS_Failure,
        kS_Max,
        kS_Nil = -1
    };

    NetCacheLoader(class FileCache *, String const &);
    ~NetCacheLoader();
    bool IsLoaded() const;
    bool HasFailed() const;
    bool IsSafeToDelete() const;
    int GetSize();
    char *GetBuffer();
    const char *GetRemotePath() const;

    State mState;
    FileCache *mCache;
    String unk8;
    FileLoader *unk10;
    int unk14;
    int unk18;
    int unk1c;
    int unk20;

protected:
    void SetState(NetCacheLoader::State);
    void WriteToCache();
    void Poll();
};
