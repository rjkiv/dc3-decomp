#include "utl/NetLoader.h"
#include "NetCacheMgr.h"
#include "Str.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"

NetLoader::NetLoader(String const &s)
    : unk4(s), mIsLoaded(0), unk10(0), unk18(-1), unk1c(0) {
    MILO_ASSERT(TheNetCacheMgr, 0x30);
}

NetLoader::~NetLoader() {
    if (unk10) {
        MemFree(unk10, "NetLoader.cpp", 0x41);
        unk10 = 0;
    }
}

char *NetLoader::DetachBuffer() {
    if (mIsLoaded == false)
        return 0;
    char *c = unk10;
    unk10 = 0;
    return c;
}

NetLoader *NetLoader::Create(String const &) { return 0; }

bool NetLoader::IsLoaded() { return mIsLoaded; }

void NetLoader::SetSize(int i) { unk18 = i; }

void NetLoader::AttachBuffer(char *c) {
    if (unk10 != 0) {
        MILO_ASSERT(mIsLoaded, 0x71);
    }
    unk10 = c;
}

void NetLoader::PostDownload() { mIsLoaded = true; }

NetLoaderStub::NetLoaderStub(String const &) {}

NetLoaderStub::~NetLoaderStub() {}

void NetLoaderStub::PollLoading() {}

DataNetLoader::DataNetLoader(String const &s) {
    if (TheNetCacheMgr == nullptr) {
        MILO_FAIL("Tried to create a DataNetLoader, but TheNetCacheMgr is NULL.\n");
    } else {
    }
}

DataNetLoader::~DataNetLoader() {}

bool DataNetLoader::IsLoaded() { return false; }

bool DataNetLoader::HasFailed() { return false; }

void DataNetLoader::PollLoading() {}
