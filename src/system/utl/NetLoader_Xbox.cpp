#include "utl/NetLoader_Xbox.h"
#include "net/HttpGet.h"
#include "os/Debug.h"
#include "utl/NetCacheMgr_Xbox.h"
#include "utl/NetCacheMgr.h"

NetLoaderXbox::NetLoaderXbox(const String &str) : NetLoader(str), unk24(0) {
    NetCacheMgrXbox *pNetCacheMgrXbox = dynamic_cast<NetCacheMgrXbox *>(TheNetCacheMgr);
    MILO_ASSERT(pNetCacheMgrXbox, 0x17);
    unsigned int ip = pNetCacheMgrXbox->GetIP();
    String root = TheNetCacheMgr->GetServerRoot();
    root += str;
    mHttpGet = new HttpGet(ip, TheNetCacheMgr->GetPort(), root.c_str(), nullptr);
    MILO_ASSERT(mHttpGet, 0x20);
}

NetLoaderXbox::~NetLoaderXbox() { RELEASE(mHttpGet); }

void NetLoaderXbox::PollLoading() {
    MILO_ASSERT(mHttpGet, 0x2A);
    mHttpGet->Poll();
    if (!unk24 && mHttpGet->IsDownloaded()) {
        unk24 = true;
        SetSize(mHttpGet->GetBufferSize());
        AttachBuffer(mHttpGet->DetachBuffer());
        PostDownload();
    } else if (mHttpGet->HasFailed()) {
        if (mHttpGet->FailType() != 3) {
            SetFailType((NetLoaderFailType)0);
        } else {
            SetFailType((NetLoaderFailType)1);
        }
    }
}

bool NetLoaderXbox::HasFailed() {
    MILO_ASSERT(mHttpGet, 0x4C);
    return mHttpGet->HasFailed();
}
