#include "utl/NetCacheMgr_Xbox.h"
#include "net/XLSPConnection.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "utl/NetCacheMgr.h"

NetCacheMgrXbox::NetCacheMgrXbox() : mIsConnected(false) {}
NetCacheMgrXbox::~NetCacheMgrXbox() {}

void NetCacheMgrXbox::Poll() {
    NetCacheMgr::Poll();
    mXLSPConnection.Poll();
    if (GetState() < 2U) {
        if (IsServerLocal()) {
            mIsConnected = true;
        } else {
            if (!mIsConnected && mXLSPConnection.GetState() == 3) {
                mIsConnected = true;
            }
            if (!HasFailed() && mXLSPConnection.GetState() == 4) {
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
    mIsConnected = false;
    if (!IsServerLocal()) {
        mXLSPConnection.Connect(
            TheNetCacheMgr->GetXLSPFilter(), TheNetCacheMgr->GetServiceId()
        );
    }
}

void NetCacheMgrXbox::UnloadInit() {
    if (!IsServerLocal()) {
        mXLSPConnection.Disconnect();
    }
}

unsigned int NetCacheMgrXbox::GetIP() {
    MILO_ASSERT(!IsServerLocal(), 0x48);
    if (mXLSPConnection.GetState() == 4) {
        MILO_NOTIFY("NetCacheMgr Error: XLSPConnection Failed");
        return 0;
    } else {
        return mXLSPConnection.GetServiceIP();
    }
}

bool NetCacheMgrXbox::IsDoneUnloading() const {
    return mXLSPConnection.GetStateDesired() == XLSPConnection::kS_Disconnected;
}
