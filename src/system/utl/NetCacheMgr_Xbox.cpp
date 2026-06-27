#include "utl/NetCacheMgr_Xbox.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/NetCacheMgr.h"

NetCacheMgrXbox::NetCacheMgrXbox() : mDoneLoading(false) {}
NetCacheMgrXbox::~NetCacheMgrXbox() {}

void NetCacheMgrXbox::Poll() {
    NetCacheMgr::Poll();
    mConnection.Poll();
    if (mState < 2U) {
        if (IsServerLocal()) {
            mDoneLoading = true;
        } else {
            if (!mDoneLoading && mConnection.GetState() == 3) {
                mDoneLoading = true;
            }
            if (!mHasFailed && mConnection.GetState() == 4) {
                if (!ThePlatformMgr.IsEthernetCableConnected()) {
                    SetFail(kNCMFT_NoEthernetCable);
                } else {
                    SetFail(kNCMFT_StoreServer);
                }
            }
        }
    }
}

void NetCacheMgrXbox::LoadInit() {
    mDoneLoading = false;
    if (!IsServerLocal()) {
        mConnection.Connect(
            TheNetCacheMgr->GetXLSPFilter(), TheNetCacheMgr->GetServiceId()
        );
    }
}

void NetCacheMgrXbox::UnloadInit() {
    if (!IsServerLocal()) {
        mConnection.Disconnect();
    }
}

unsigned int NetCacheMgrXbox::GetIP() {
    MILO_ASSERT(!IsServerLocal(), 0x48);
    if (mConnection.GetState() == 4) {
        MILO_NOTIFY("NetCacheMgr Error: XLSPConnection Failed");
        return 0;
    } else {
        return mConnection.GetServiceIP();
    }
}

bool NetCacheMgrXbox::IsDoneUnloading() const { return mConnection.GetUnk8() == 0; }
