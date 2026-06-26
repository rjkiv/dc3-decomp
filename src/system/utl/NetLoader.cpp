#include "utl/NetLoader.h"
#include "NetCacheMgr.h"
#include "Str.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/File.h"
#include "utl/BufStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/NetCacheMgr.h"
#include "utl/NetLoader_Xbox.h"

const float NetLoaderStub::kNetSimKbPerSecond = 32.0f;
const float NetLoaderStub::kNetSimInitialDelay = 0.2f;

NetLoader::NetLoader(const String &s)
    : mStrRemotePath(s), mIsLoaded(0), mBuffer(0), mSize(-1),
      mFailType((NetLoaderFailType)0) {
    MILO_ASSERT(TheNetCacheMgr, 0x30);
}

NetLoader::~NetLoader() {
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x41);
        mBuffer = 0;
    }
}

void NetLoader::AttachBuffer(char *buffer) {
    if (mBuffer != 0) {
        MILO_ASSERT(mIsLoaded, 0x71);
        if (mBuffer) {
            MemFree(mBuffer, __FILE__, 0x72);
            mBuffer = nullptr;
        }
    }
    mBuffer = buffer;
}

char *NetLoader::DetachBuffer() {
    if (!mIsLoaded) {
        return nullptr;
    } else {
        char *old = mBuffer;
        mBuffer = nullptr;
        return old;
    }
}

NetLoader *NetLoader::Create(const String &str) {
    if (TheNetCacheMgr->IsServerLocal()) {
        return new NetLoaderStub(str);
    } else {
        return new NetLoaderXbox(str);
    }
}

bool NetLoader::IsLoaded() { return mIsLoaded; }
int NetLoader::GetSize() { return mSize; }
void NetLoader::SetSize(int size) { mSize = size; }
void NetLoader::PostDownload() { mIsLoaded = true; }
const char *NetLoader::GetRemotePath() const { return mStrRemotePath.c_str(); }
void NetLoader::SetFailType(NetLoaderFailType ft) { mFailType = ft; }

NetLoaderStub::NetLoaderStub(const String &str) : NetLoader(str), mFileLoader(nullptr) {
    FilePath path(
        MakeString("%s/%s", TheNetCacheMgr->GetServerRoot(), mStrRemotePath.c_str())
    );
    mFileLoader =
        new FileLoader(path, path.c_str(), kLoadFront, 0, false, true, nullptr, nullptr);
    MILO_ASSERT(mFileLoader, 0xA0);
    float size = mFileLoader->GetSize() * 0.0009765625f;
    mNetSimEndTime = size * 0.03125f + TheTaskMgr.UISeconds() + kNetSimInitialDelay;
}

NetLoaderStub::~NetLoaderStub() { RELEASE(mFileLoader); }

void NetLoaderStub::PollLoading() {
    MILO_ASSERT(mFileLoader, 0xB0);
    if (!mIsLoaded) {
        if (!mFileLoader->IsLoaded()) {
            TheLoadMgr.Poll();
        }
        if (mFileLoader->IsLoaded() && mNetSimEndTime <= TheTaskMgr.UISeconds()) {
            int size = -1;
            AttachBuffer(mFileLoader->GetBuffer(&size));
            mIsLoaded = true;
            mSize = size;
        }
    }
}

DataNetLoader::DataNetLoader(const String &str) : mNetLoader(0), mData(0) {
    if (!TheNetCacheMgr) {
        MILO_FAIL("Tried to create a DataNetLoader, but TheNetCacheMgr is NULL.\n");
    } else {
        mNetLoader = TheNetCacheMgr->AddNetLoader(str.c_str(), kNetLoaderPosNext);
    }
}

DataNetLoader::~DataNetLoader() {
    if (mNetLoader) {
        TheNetCacheMgr->DeleteNetLoader(mNetLoader);
        mNetLoader = nullptr;
    }
    if (mData) {
        mData->Release();
        mData = nullptr;
    }
}

void DataNetLoader::PollLoading() {
    if (mNetLoader) {
        if (mNetLoader->IsLoaded()) {
            int size = mNetLoader->GetSize();
            char *buffer = mNetLoader->GetBuffer();
            const char *remotePath = mNetLoader->GetRemotePath();
            if (streq(FileGetExt(remotePath), "dtz")) {
                DataArray::SetFile(remotePath);
                mData = LoadDtz(buffer, size);
            } else {
                BufStream bs(buffer, size, true);
                mData = DataReadStream(&bs);
            }
            TheNetCacheMgr->DeleteNetLoader(mNetLoader);
            mNetLoader = nullptr;
        } else if (mNetLoader->HasFailed()) {
            TheNetCacheMgr->DeleteNetLoader(mNetLoader);
            mNetLoader = nullptr;
        }
    }
}

bool DataNetLoader::HasFailed() {
    if (mNetLoader) {
        return mNetLoader->HasFailed();
    } else
        return !mData;
}

bool DataNetLoader::IsLoaded() {
    bool netLoaded = true;
    if (mNetLoader)
        netLoaded = mNetLoader->IsLoaded();
    return netLoaded && mData;
}
