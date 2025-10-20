#include "utl/NetCacheLoader.h"
#include "NetCacheMgr.h"
#include "os/Debug.h"
#include "os/FileCache.h"
#include "utl/Loader.h"
#include "utl/Str.h"

NetCacheLoader::NetCacheLoader(FileCache *f, String const &s)
    : mState(kS_Nil), mCache(f), unk8(s) {
    bool b = TheNetCacheMgr->IsLocalFile(s.c_str());
    MILO_ASSERT(mState != kS_Nil, 0x28);
    MILO_ASSERT(mCache, 0x2b);
}

NetCacheLoader::~NetCacheLoader() { SetState(kS_Nil); }

bool NetCacheLoader::IsLoaded() const { return false; }

bool NetCacheLoader::HasFailed() const { return false; }

bool NetCacheLoader::IsSafeToDelete() const { return false; }

int NetCacheLoader::GetSize() {
    if (unk10) {
        return unk10->GetSize();
    }
}

char *NetCacheLoader::GetBuffer() { return 0; }

void NetCacheLoader::SetState(NetCacheLoader::State) {}

void NetCacheLoader::WriteToCache() {}

void NetCacheLoader::Poll() {}

const char *NetCacheLoader::GetRemotePath() const { return 0; }
