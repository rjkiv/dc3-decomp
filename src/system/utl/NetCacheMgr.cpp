#include "utl/NetCacheMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/MemMgr.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetLoader.h"
#include "utl/Symbol.h"

NetCacheMgr *TheNetCacheMgr;

NetCacheMgr::NetCacheMgr()
    : unk2c(-1), unk30(0), unk34(kNCMFT_Unknown), unk40(0), unk44(0), unk50(),
      mLoadCacheSize(0), mCache(0), unk64(0) {
    SetName("net_cache_mgr", ObjectDir::Main());
}

NetCacheMgr::~NetCacheMgr() {}

void NetCacheMgr::Poll() {}

bool NetCacheMgr::IsUnloaded() const { return unk2c - 2; }

bool NetCacheMgr::IsReady() const { return (unk2c == 1 && !unk30 && unk64 == 1); }

bool NetCacheMgr::IsLocalFile(char const *) const { return false; }

void NetCacheMgr::DeleteNetLoader(NetLoader *nl) {
    if (nl == nullptr)
        return;
}

unsigned short NetCacheMgr::GetPort() const { return 0; }

char const *NetCacheMgr::GetServerRoot() const { return 0; }

bool NetCacheMgr::IsServerLocal() const { return false; }

bool NetCacheMgr::IsDebug() const { return false; }

Symbol NetCacheMgr::CheatNextServer() { return 0; }

void NetCacheMgr::Unload() {
    unk64 -= 1;
    if (unk64 - 1 < 0) {
        MILO_NOTIFY("NetCacheMgr::Unload() called more times than NetCacheMgr::Load()!\n");
        unk64 = 0;
    } else {
        SetState(kNCMS_UnloadWaitForWrite);
    }
}

void NetCacheMgr::Load(NetCacheMgr::CacheSize) {}

void NetCacheMgr::SetFail(NetCacheMgrFailType n) {
    unk30 = true;
    unk34 = n;
}

void NetCacheMgr::SetState(NetCacheMgrState state) {}

void NetCacheMgr::OnInit(DataArray *) {}

void NetCacheMgr::PollLoaders() {}

void NetCacheMgr::EnterLoadState() {
    if (!unk30) {
        MILO_ASSERT(!mCache, 0x2ab);
        MILO_ASSERT(mLoadCacheSize, 0x2ac);

        mLoadCacheSize = 0;
    }
}

bool NetCacheMgr::IsUnloadStateDone() const {
    if (unk6c == false)
        return false;
    return true;
}

void NetCacheMgr::EnterUnloadState() {}

void NetCacheMgr::DeleteNetCacheLoader(NetCacheLoader *n) {}

// NetCacheMgr::ServerData const &NetCacheMgr::Server() const { return new ServerData(); }

void NetCacheMgrTerminate() {
    delete TheNetCacheMgr;
    TheNetCacheMgr = nullptr;
}

void NetCacheMgrInit() {
    MILO_ASSERT(TheNetCacheMgr == NULL, 0x1f);
    TheNetCacheMgr = new NetCacheMgr(); // needs to be new NetCacheXbox() later
}

void NetLoaderRef::Poll() {}

bool NetLoaderRef::NeedsToDownload() { return false; }

bool NetLoaderRef::IsDownloading() { return false; }

bool NetLoaderRef::IsLoadedOrFailed() { return false; }

bool NetLoaderRef::IsSafeToDelete() { return false; }

void NetLoaderRef::DeleteLoader() {}

BEGIN_HANDLERS(NetCacheMgr)
END_HANDLERS
