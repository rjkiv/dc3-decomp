#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/FileCache.h"
#include "utl/Cache.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetLoader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

class NetCacheLoader;

enum NetCacheMgrFailType {
    kNCMFT_Unknown,
    kNCMFT_StoreServer,
    kNCMFT_ClientError,
    kNCMFT_NoEthernetCable,
    kNCMFT_Max
};

enum NetCacheMgrState {
    kNCMS_Load = 0x0000,
    kNCMS_Ready = 0x0001,
    kNCMS_Unload = 0x0002,
    kNCMS_Max = 0x0003,
    kNCMS_Nil = -1
};

enum LoadState {
    kLS_None,
    kLS_Mount,
    kLS_Delete,
    kLS_ReMount,
    kLS_Resync
};

enum NetLoaderPos {
    kNetLoaderPosNext = 0x0000,
    kNetLoaderPosBack = 0x0001,
};

struct NetLoaderRef {
    NetLoaderRef() : mCount(0), mNetLoader(nullptr), mCacheLoader(nullptr) {}
    NetLoaderRef(
        const String &name, int count, NetLoader *netLoader, NetCacheLoader *cacheLoader
    )
        : mName(name), mCount(count), mNetLoader(netLoader), mCacheLoader(cacheLoader) {}
    void Poll();
    bool NeedsToDownload();
    bool IsDownloading();
    bool IsLoadedOrFailed();
    bool IsSafeToDelete();
    void DeleteLoader();
    bool IsValid() const;
    void AddRef() { mCount++; }
    void ReleaseRef() { mCount--; }

    String mName; // 0x0
    int mCount; // 0x8
    NetLoader *mNetLoader; // 0xc
    NetCacheLoader *mCacheLoader; // 0x10
};

class NetCacheMgr : public Hmx::Object {
public:
    // size 0x18
    struct ServerData {
        Symbol type; // 0x0
        bool local; // 0x4
        const char *server; // 0x8
        unsigned short port; // 0xc
        const char *root; // 0x10
        bool debug; // 0x14
        bool verifySSL; // 0x15
    };

    enum RefType {
    };

    enum CacheSize {
        kCacheSize_Small = 0x0000,
        kCacheSize_Large = 0x0001,
        kCacheSize_Huge = 0x0002,
    };

    NetCacheMgr();
    // Hmx::Object
    virtual ~NetCacheMgr();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();

    unsigned int GetServiceId() const;
    NetCacheMgrFailType GetFailType() const;
    const char *GetXLSPFilter() const;
    bool IsUnloaded() const;
    bool IsReady() const;
    bool IsLocalFile(const char *) const;
    void DeleteNetLoader(NetLoader *);
    unsigned short GetPort() const;
    char const *GetServerRoot() const;
    bool IsServerLocal() const;
    bool IsDebug() const;
    Symbol CheatNextServer();
    void Unload();
    void Load(NetCacheMgr::CacheSize);
    void DeleteNetCacheLoader(NetCacheLoader *);
    NetLoader *AddNetLoader(const char *, NetLoaderPos);
    NetCacheLoader *AddNetCacheLoader(const char *, NetLoaderPos);

    bool HasFailed() const { return mHasFailed; }

private:
    void EnterLoadState();
    bool IsUnloadStateDone() const;
    void EnterUnloadState();
    NetCacheMgr::ServerData const &Server() const;

protected:
    virtual void LoadInit() {}
    virtual bool IsDoneLoading() const { return true; }
    virtual void ReadyInit() {}
    virtual void UnloadInit() {}
    virtual bool IsDoneUnloading() const { return true; }

    void SetFail(NetCacheMgrFailType);
    void SetState(NetCacheMgrState);
    void OnInit(DataArray *);
    void PollLoaders();
    void DebugClearCache();
    NetLoaderRef *AddLoaderRef(const char *, RefType, NetLoaderPos);

    NetCacheMgrState mState; // 0x2c
    bool mHasFailed; // 0x30
    NetCacheMgrFailType mFailType; // 0x34
    String mXLSPFilter; // 0x38
    unsigned int mServiceId; // 0x40
    bool unk44;
    std::list<ServerData> mServers; // 0x48
    Symbol mServerType; // 0x50
    int mLoadCacheSize; // 0x54
    FileCache *mCache; // 0x58
    std::list<NetLoaderRef> mNetLoaderRefs; // 0x5c
    int mLoadCount; // 0x64
};

void NetCacheMgrTerminate();
void NetCacheMgrInit();

extern NetCacheMgr *TheNetCacheMgr;
