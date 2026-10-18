#pragma once
#include "utl/NetCacheMgr.h"
#include "net/XLSPConnection.h"

class NetCacheMgrXbox : public NetCacheMgr {
public:
    NetCacheMgrXbox();
    virtual ~NetCacheMgrXbox();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();

    unsigned int GetIP();

protected:
    virtual void LoadInit();
    virtual bool IsDoneLoading() const { return mIsConnected; }
    virtual void UnloadInit();
    virtual bool IsDoneUnloading() const;

    bool mIsConnected; // 0x68
    XLSPConnection mXLSPConnection; // 0x70
};
