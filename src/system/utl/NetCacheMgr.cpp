#include "utl/NetCacheMgr.h"
#include "NetCacheMgr_Xbox.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/FileCache.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/NetCacheLoader.h"
#include "utl/NetLoader.h"
#include "utl/Symbol.h"

NetCacheMgr *TheNetCacheMgr;

#pragma region NetCacheMgr

NetCacheMgr::NetCacheMgr()
    : mState(kNCMS_Nil), mHasFailed(0), mFailType(kNCMFT_Unknown), mServiceId(0),
      unk44(0), mLoadCacheSize(0), mCache(0), mLoadCount(0) {
    SetName("net_cache_mgr", ObjectDir::Main());
}

NetCacheMgr::~NetCacheMgr() {}

BEGIN_HANDLERS(NetCacheMgr)
    HANDLE_ACTION(init, OnInit(_msg->Array(2)))
    HANDLE_ACTION(debug_clear_cache, DebugClearCache())
    HANDLE_EXPR(cheat_next_server, CheatNextServer())
    HANDLE_EXPR(server_type, mServerType)
    HANDLE_EXPR(is_local, IsLocalFile(_msg->Str(2)))
END_HANDLERS

void NetCacheMgr::Poll() {
    if (!unk44) {
        unk44 = ThePlatformMgr.GetServiceID("store", mServiceId);
    }
    PollLoaders();
    switch (mState) {
    case kNCMS_Load:
        if (IsDoneLoading()) {
            SetState(kNCMS_Ready);
        }
        break;
    case kNCMS_Unload:
        if (IsUnloadStateDone()) {
            SetState(kNCMS_Nil);
        }
        break;
    default:
        break;
    }
}

unsigned int NetCacheMgr::GetServiceId() const { return mServiceId; }
NetCacheMgrFailType NetCacheMgr::GetFailType() const { return mFailType; }
const char *NetCacheMgr::GetXLSPFilter() const { return mXLSPFilter.c_str(); }

void NetCacheMgr::DebugClearCache() {
    if (IsReady()) {
        mCache->Clear();
    }
}

bool NetCacheMgr::IsUnloaded() const { return mState != kNCMS_Unload; }
bool NetCacheMgr::IsReady() const {
    return (mState == kNCMS_Ready && !mHasFailed && mLoadCount == 1);
}

bool NetCacheMgr::IsLocalFile(const char *file) const {
    if (!IsReady()) {
        return false;
    } else
        return mCache->FileCached(file);
}

void NetCacheMgr::SetFail(NetCacheMgrFailType n) {
    mHasFailed = true;
    mFailType = n;
}

void NetCacheMgr::EnterLoadState() {
    mHasFailed = false;
    LoadInit();
    if (!mHasFailed) {
        MILO_ASSERT(!mCache, 0x2ab);
        MILO_ASSERT(mLoadCacheSize, 0x2ac);
        mCache = new FileCache(mLoadCacheSize, kLoadStayBack, true, false);
        mLoadCacheSize = 0;
    }
}

void NetCacheMgr::EnterUnloadState() {
    UnloadInit();
    FOREACH (it, mNetLoaderRefs) {
        NetLoaderRef &cur = *it;
        if (cur.mCount > 0) {
            MILO_NOTIFY(
                "Loader for %s has %d reference(s) left unaccounted for!",
                cur.mName,
                cur.mCount
            );
            cur.mCount = 0;
        }
    }
    RELEASE(mCache);
}

bool NetCacheMgr::IsUnloadStateDone() const {
    return IsDoneUnloading() && mNetLoaderRefs.empty();
}

void NetCacheMgr::DeleteNetLoader(NetLoader *nl) {
    if (nl) {
        FOREACH (it, mNetLoaderRefs) {
            if (it->mNetLoader == nl) {
                it->mCount--;
                return;
            }
        }
    }
}

void NetCacheMgr::DeleteNetCacheLoader(NetCacheLoader *ncl) {
    if (ncl) {
        FOREACH (it, mNetLoaderRefs) {
            if (it->mCacheLoader == ncl) {
                it->mCount--;
                return;
            }
        }
    }
}

const NetCacheMgr::ServerData &NetCacheMgr::Server() const {
    auto s = mServers.begin();
    for (; s != mServers.end() && s->type != mServerType; s++)
        ;
    MILO_ASSERT(s != mServers.end(), 0x2D7);
    return *s;
}

unsigned short NetCacheMgr::GetPort() const { return Server().port; }
const char *NetCacheMgr::GetServerRoot() const { return Server().root; }
bool NetCacheMgr::IsServerLocal() const { return Server().local; }
bool NetCacheMgr::IsDebug() const { return Server().debug; }

Symbol NetCacheMgr::CheatNextServer() {
    auto s = mServers.begin();
    for (; s != mServers.end() && s->type != mServerType; s++)
        ;
    MILO_ASSERT(s != mServers.end(), 0x22B);
    if (s == mServers.end()) {
        s = mServers.begin();
    }
    mServerType = s->type;
    static Symbol local("local");
    if (UsingCD() && mServerType == local) {
        CheatNextServer();
    }
    return mServerType;
}

void NetCacheMgr::Load(NetCacheMgr::CacheSize cs) {
    if (mLoadCount == 0) {
        while (mState == 2) {
            NetCacheMgr::Poll();
        }
    }
    mLoadCount++;
    MILO_ASSERT(mLoadCount <= 2, 0x120);
    if (mState == kNCMS_Load && !mHasFailed) {
        MILO_NOTIFY("NetCcaheMgr::Load() called before previous load had finished.");
    }
    mLoadCacheSize = cs == kCacheSize_Small ? 0x100000 : 0x500000;
    if (mLoadCount == 1) {
        SetState(kNCMS_Load);
    }
}

void NetCacheMgr::Unload() {
    mLoadCount--;
    if (mLoadCount < 0) {
        MILO_NOTIFY("NetCacheMgr::Unload() called more times than NetCacheMgr::Load()!\n");
        mLoadCount = 0;
    } else {
        SetState(kNCMS_Unload);
    }
}

NetLoader *NetCacheMgr::AddNetLoader(const char *cc, NetLoaderPos pos) {
    NetLoaderRef *pNetLoaderRef = AddLoaderRef(cc, (RefType)1, pos);
    if (!pNetLoaderRef)
        return nullptr;
    else {
        MILO_ASSERT(pNetLoaderRef && pNetLoaderRef->mNetLoader, 0x160);
        return pNetLoaderRef->mNetLoader;
    }
}

NetCacheLoader *NetCacheMgr::AddNetCacheLoader(const char *cc, NetLoaderPos pos) {
    NetLoaderRef *pNetLoaderRef = AddLoaderRef(cc, (RefType)0, pos);
    if (!pNetLoaderRef)
        return nullptr;
    else {
        MILO_ASSERT(pNetLoaderRef && pNetLoaderRef->mCacheLoader, 0x14F);
        return pNetLoaderRef->mCacheLoader;
    }
}

void NetCacheMgr::SetState(NetCacheMgrState state) {
    if (mState != state) {
        while (true) {
            if (mState == 2) {
                mHasFailed = false;
            }
            if (mState == kNCMS_Nil && state == kNCMS_Unload) {
                MILO_FAIL(
                    "NetCacheMgr attempted to move straight from kNCMS_Nil to kNCMS_Unload!\n"
                );
            }
            mState = state;
            if (state != -1)
                break;
            MILO_ASSERT(mNetLoaderRefs.empty(), 0x28B);
            if (mLoadCount <= 0)
                return;
            state = kNCMS_Load;
            if (mState == 0)
                return;
        }
        switch (state) {
        case 0:
            EnterLoadState();
            break;
        case 1:
            ReadyInit();
            break;
        case 2:
            EnterUnloadState();
            break;
        default:
            break;
        }
    }
}

void NetCacheMgr::OnInit(DataArray *pData) {
    MILO_ASSERT(pData, 0x46);
    static Symbol xlsp_service_id("xlsp_service_id");
    static Symbol xlsp_filter("xlsp_filter");
    mServiceId = 0;
    mXLSPFilter = pData->FindStr(xlsp_filter);
    static Symbol servers("servers");
    DataArray *serverArr = pData->FindArray(servers);
    static Symbol server("server");
    static Symbol port("port");
    static Symbol root("root");
    static Symbol local("local");
    MILO_ASSERT(mServers.empty(), 0x56);
    for (int i = 1; i < serverArr->Size(); i++) {
        ServerData serverData;
        DataArray *curArr = serverArr->Array(i);
        serverData.type = curArr->Sym(0);
        serverData.port = 0;
        serverData.server = gNullStr;
        static Symbol debug("debug");
        serverData.debug = false;
        curArr->FindData(debug, serverData.debug, false);
        static Symbol verify_ssl("verify_ssl");
        bool vSSL = true;
        curArr->FindData(verify_ssl, vSSL, false);
        bool isLocal = false;
        serverData.verifySSL = vSSL;
        curArr->FindData(local, isLocal, false);
        const char *serverStr = nullptr;
        serverData.local = isLocal;
        curArr->FindData(server, serverStr, false);
        serverData.server = serverStr;
        int serverPort = 0;
        curArr->FindData(port, serverPort, false);
        serverData.port = serverPort;
        serverData.root = curArr->FindStr(root);
        mServers.push_back(serverData);
    }
    static Symbol default_server("default_server");
    mServerType = pData->FindSym(default_server);
    FOREACH (it, mServers) {
        // ok then
    }
}

void NetCacheMgr::PollLoaders() {
    bool b = true;
    auto it = mNetLoaderRefs.begin();
    while (it != mNetLoaderRefs.end()) {
        NetLoaderRef &netLoaderRef = *it;
        MILO_ASSERT(netLoaderRef.IsValid(), 0xe9);
        if (!netLoaderRef.NeedsToDownload() || netLoaderRef.IsLoadedOrFailed()) {
            netLoaderRef.Poll();
        } else if (b) {
            netLoaderRef.Poll();
            b = false;
        }

        if (netLoaderRef.mCount < 1 && netLoaderRef.IsSafeToDelete()) {
            netLoaderRef.DeleteLoader();
            it = mNetLoaderRefs.erase(it);
        } else {
            ++it;
        }
    }
}

NetLoaderRef *NetCacheMgr::AddLoaderRef(const char *cc1, RefType rt, NetLoaderPos pos) {
    if (*cc1 && IsReady()) {
        NetLoaderRef *pNetLoaderRef = nullptr;
        FOREACH (it, mNetLoaderRefs) {
            NetLoaderRef &ref = *it;
            if (stricmp(it->mName.c_str(), cc1) == 0) {
                if (rt == 0 && ref.mCacheLoader) {
                    MILO_ASSERT(ref.mNetLoader == NULL, 0x17A);
                    pNetLoaderRef = &ref;
                    break;
                } else if (rt == 1 && ref.mNetLoader) {
                    MILO_ASSERT(ref.mCacheLoader == NULL, 0x180);
                    pNetLoaderRef = &ref;
                    break;
                } else {
                    MILO_LOG(
                        "Found loader for %s, but it was not type %d.\n",
                        ref.mName.c_str(),
                        rt
                    );
                }
            }
        }
        NetLoaderRef ref;
        if (!pNetLoaderRef) {
            switch (rt) {
            case 0: {
                NetCacheLoader *ncl = new NetCacheLoader(mCache, cc1);
                ref = NetLoaderRef(cc1, 0, nullptr, ncl);
                break;
            }
            case 1: {
                NetLoader *nl = NetLoader::Create(cc1);
                ref = NetLoaderRef(cc1, 0, nl, nullptr);
                break;
            }
            default:
                MILO_FAIL("Unknown ref type %d.\n", rt);
                break;
            }
            switch (pos) {
            case kNetLoaderPosNext: {
                auto it = mNetLoaderRefs.begin();
                for (; it != mNetLoaderRefs.end(); ++it) {
                    if (!it->IsDownloading() && !it->IsLoadedOrFailed()) {
                        break;
                    }
                }
                it = mNetLoaderRefs.insert(it, ref);
                pNetLoaderRef = &*it;
                break;
            }
            case kNetLoaderPosBack: {
                mNetLoaderRefs.push_back(ref);
                pNetLoaderRef = &mNetLoaderRefs.back();
                break;
            }
            default:
                MILO_FAIL("Unknown net loader pos %d.\n", pos);
                break;
            }
        }
        MILO_ASSERT(pNetLoaderRef, 0x1C2);
        pNetLoaderRef->AddRef();
        return pNetLoaderRef;
    } else {
        return nullptr;
    }
}

#pragma endregion
#pragma region NetLoaderRef

void NetLoaderRef::Poll() {
    MILO_ASSERT(IsValid(), 0x315);
    if (mCacheLoader) {
        mCacheLoader->PollLoading();
    } else {
        mNetLoader->PollLoading();
    }
}

bool NetLoaderRef::IsSafeToDelete() {
    MILO_ASSERT(IsValid(), 0x334);
    if (mCacheLoader) {
        return mCacheLoader->IsSafeToDelete();
    } else {
        return mNetLoader->IsSafeToDelete();
    }
}

void NetLoaderRef::DeleteLoader() {
    MILO_ASSERT(IsSafeToDelete(), 0x33A);
    RELEASE(mCacheLoader);
    RELEASE(mNetLoader);
}

bool NetLoaderRef::IsDownloading() {
    MILO_ASSERT(IsValid(), 0x321);
    return !mCacheLoader || mCacheLoader->GetState() == 2;
}

bool NetLoaderRef::IsValid() const {
    // mCacheLoader XOR mNetLoader
    return (!mCacheLoader || !mNetLoader) && (mCacheLoader || mNetLoader);
}

bool NetLoaderRef::NeedsToDownload() {
    MILO_ASSERT(IsValid(), 0x31B);
    if (mCacheLoader) {
        bool stateCheck = mCacheLoader->GetState() == 1 || mCacheLoader->GetState() == 2;
        if (!stateCheck) {
            return false;
        }
    }
    return true;
}

bool NetLoaderRef::IsLoadedOrFailed() {
    MILO_ASSERT(IsValid(), 0x327);
    if (mCacheLoader) {
        return mCacheLoader->IsLoaded() || mCacheLoader->HasFailed();
    } else {
        return mNetLoader->IsLoaded() || mNetLoader->HasFailed();
    }
}

#pragma endregion

void NetCacheMgrInit() {
    MILO_ASSERT(TheNetCacheMgr == NULL, 0x1f);
    TheNetCacheMgr = new NetCacheMgrXbox();
}

void NetCacheMgrTerminate() { RELEASE(TheNetCacheMgr); }
