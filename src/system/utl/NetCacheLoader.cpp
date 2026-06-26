#include "utl/NetCacheLoader.h"
#include "NetCacheMgr.h"
#include "os/Debug.h"
#include "os/FileCache.h"
#include "utl/Loader.h"
#include "utl/NetCacheMgr.h"
#include "utl/NetLoader.h"
#include "utl/Str.h"

NetCacheLoader::NetCacheLoader(FileCache *f, const String &s)
    : mState(kS_Nil), mCache(f), mRemotePath(s), mFileLoader(0), mFileLoaderBuffer(0),
      mNetLoader(0), mNetLoaderBuffer(0), unk20(0) {
    if (TheNetCacheMgr->IsLocalFile(mRemotePath.c_str())) {
        SetState((NetCacheLoader::State)0);
    } else {
        SetState((NetCacheLoader::State)1);
    }
    MILO_ASSERT(mState != kS_Nil, 0x28);
    MILO_ASSERT(mCache, 0x2b);
}

NetCacheLoader::~NetCacheLoader() { SetState(kS_Nil); }

bool NetCacheLoader::IsLoaded() const { return mState == 3; }
bool NetCacheLoader::HasFailed() const { return mState == 4; }

bool NetCacheLoader::IsSafeToDelete() const {
    if (mNetLoader) {
        return mNetLoader->IsSafeToDelete();
    } else
        return true;
}

int NetCacheLoader::GetSize() {
    if (mNetLoader) {
        return mNetLoader->GetSize();
    } else if (mFileLoader) {
        return mFileLoader->GetSize();
    } else
        return 0;
}

char *NetCacheLoader::GetBuffer() {
    if (mNetLoader) {
        return mNetLoaderBuffer;
    } else if (mFileLoader) {
        return mFileLoaderBuffer;
    } else
        return nullptr;
}

void NetCacheLoader::SetState(NetCacheLoader::State state) {
    if (mState == state) {
        return;
    } else {
        switch (mState) {
        case 0:
            if (state != 3) {
                RELEASE(mFileLoader);
                RELEASE(mFileLoaderBuffer);
            }
            break;
        case 2:
            if (state != 3) {
                MILO_ASSERT(!mNetLoader || mNetLoader->IsSafeToDelete(), 0xC1);
                RELEASE(mNetLoader);
            }
            break;
        case 3:
            MILO_ASSERT(!mNetLoader || mNetLoader->IsSafeToDelete(), 0xC7);
            RELEASE(mNetLoader);
            mNetLoaderBuffer = nullptr;
            RELEASE(mFileLoader);
            RELEASE(mFileLoaderBuffer);
        default:
            break;
        }
    }
    mState = state;
    switch (mState) {
    case 0: {
        MILO_ASSERT(!mFileLoader, 0xE3);
        const char *strPath = GetRemotePath();
        MILO_ASSERT(TheNetCacheMgr->IsLocalFile(strPath), 0xE6);
        mFileLoader = new FileLoader(
            strPath, strPath, kLoadFront, 0, false, true, nullptr, nullptr
        );
        break;
    }
    case 2: {
        MILO_ASSERT(!mNetLoader, 0xEF);
        mNetLoader = NetLoader::Create(mRemotePath);
        break;
    }
    }
}

void NetCacheLoader::WriteToCache() {
    if (!TheNetCacheMgr->IsReady()) {
        unk20 = 0;
        SetState((NetCacheLoader::State)4);
    } else {
        MILO_ASSERT(!mNetLoaderBuffer, 0x103);
        mNetLoaderBuffer = mNetLoader->DetachBuffer();
        mCache->Add(mRemotePath.c_str(), mNetLoaderBuffer, mNetLoader->GetSize());
    }
}

void NetCacheLoader::Poll() {
    switch (mState) {
    case 0: {
        MILO_ASSERT(mFileLoader, 0x4E);
        if (mFileLoader->IsLoaded()) {
            MILO_ASSERT(!mFileLoaderBuffer, 0x52);
            mFileLoaderBuffer = mFileLoader->GetBuffer(nullptr);
            SetState((NetCacheLoader::State)3);
        }
        break;
    }
    case 1: {
        SetState((NetCacheLoader::State)2);
        break;
    }
    case 2: {
        mNetLoader->PollLoading();
        if (mNetLoader->IsLoaded()) {
            WriteToCache();
            SetState((NetCacheLoader::State)3);
        } else if (mNetLoader->HasFailed()) {
            if (mNetLoader->GetFailType() == 1) {
                MILO_LOG(
                    "Failed to find file on server: %s\n", mNetLoader->GetRemotePath()
                );
                unk20 = 2;
            } else {
                MILO_NOTIFY(
                    "NetCacheLoader failed for file: %s", mNetLoader->GetRemotePath()
                );
                unk20 = 1;
            }
            SetState((NetCacheLoader::State)4);
        }
        break;
    }
    default:
        break;
    }
}

const char *NetCacheLoader::GetRemotePath() const { return mRemotePath.c_str(); }
