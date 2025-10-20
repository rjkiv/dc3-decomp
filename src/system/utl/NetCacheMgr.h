#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Cache.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetLoader.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <list>

enum NetCacheMgrFailType {
    kNCMFT_Unknown,
    kNCMFT_StoreServer,
    kNCMFT_NoSpace,
    kNCMFT_StorageDeviceMissing,
    kNCMFT_Max
};

enum NetCacheMgrState {
    kNCMS_Load,
    kNCMS_Ready,
    kNCMS_UnloadWaitForWrite,
    kNCMS_UnloadUnmount,
    kNCMS_Failure,
    kNCMS_Max,
    kNCMS_Nil = -1
};

enum LoadState {
    kLS_None,
    kLS_Mount,
    kLS_Delete,
    kLS_ReMount,
    kLS_Resync
};

struct NetLoaderRef {
public:
    void Poll();
    bool NeedsToDownload();
    bool IsDownloading();
    bool IsLoadedOrFailed();
    bool IsSafeToDelete();
    void DeleteLoader();
};

class NetCacheMgr : public Hmx::Object {
public:
    struct ServerData {
        bool unk4;
    };

    enum RefType {
    };

    enum CacheSize {
    };

    // Hmx::Object
    virtual ~NetCacheMgr();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();

    NetCacheMgr();
    bool IsUnloaded() const;
    bool IsReady() const;
    bool IsLocalFile(char const *) const;
    // void DeleteNetCacheLoader(NetCacheLoader *);
    void DeleteNetLoader(NetLoader *);
    unsigned short GetPort() const;
    char const *GetServerRoot() const;
    bool IsServerLocal() const;
    bool IsDebug() const;
    Symbol CheatNextServer();
    void Unload();
    void Load(NetCacheMgr::CacheSize);
    void DeleteNetCacheLoader(NetCacheLoader *);

    int unk2c;
    bool unk30;
    NetCacheMgrFailType unk34;
    String unk38;
    int unk40;
    bool unk44;
    std::list<NetCacheMgr::ServerData> unk48;
    Symbol unk50;
    int mLoadCacheSize; // 0x54
    Cache *mCache; // 0x58
    std::list<NetLoaderRef> unk5c;
    int unk64;
    u32 unk68;
    bool unk6c;

protected:
    void SetFail(NetCacheMgrFailType);
    void SetState(NetCacheMgrState);
    void OnInit(DataArray *);
    void PollLoaders();
    // NetLoaderRef *AddLoaderRef(char const *, NetCacheMgr::RefType, NetLoaderPos);
    // NetLoader *AddNetLoader(char const *, NetLoaderPos);

private:
    void EnterLoadState();
    bool IsUnloadStateDone() const;
    void EnterUnloadState();
    NetCacheMgr::ServerData const &Server() const;
};

void NetCacheMgrTerminate();
void NetCacheMgrInit();

extern NetCacheMgr *TheNetCacheMgr;
