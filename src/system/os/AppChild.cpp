#include "os/AppChild.h"
#include "HolmesClient.h"
#include "NetStream.h"
#include "NetworkSocket.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Option.h"
#include "obj/DataFunc.h"

AppChild *TheAppChild;

AppChild::~AppChild() { delete mStream; }

AppChild::AppChild(const char *str) : mEnabled(1), mStream(0), mSync(0) {
    NetAddress addr(HolmesResolveIP().GetIP(), 0x11BF);
    NetStream *stream = new NetStream();
    stream->ClientConnect(addr);
    mStream = stream;
    MILO_LOG("AppChild::Connect\n");
}

void AppChild::Sync() {
    short lol = 1;
    *mStream << lol;
    mStream->Flush();
    mSync = true;
}

void AppChild::Sync(unsigned short sh) {
    *mStream << sh;
    mStream->Flush();
    mSync = true;
}

DataNode EnableAppChild(DataArray *) {
    if (TheAppChild)
        TheAppChild->SetEnabled(true);
    return 0;
}

DataNode DisableAppChild(DataArray *) {
    if (TheAppChild)
        TheAppChild->SetEnabled(false);
    return 0;
}

DataNode SyncAppChild(DataArray *) {
    if (TheAppChild)
        TheAppChild->Sync();
    return 0;
}

void AppChild::Init() {
    bool appchildbool = OptionBool("app_child", false);
    if (appchildbool) {
        MILO_ASSERT(!TheAppChild, 0x3C);
        TheAppChild = new AppChild(OptionStr("pipe_name", 0));
    }
    DataVariable("app_child") = appchildbool;
    DataRegisterFunc("enable_app_child", EnableAppChild);
    DataRegisterFunc("disable_app_child", DisableAppChild);
    DataRegisterFunc("sync_app_child", SyncAppChild);
}

void AppChild::Terminate() { RELEASE(TheAppChild); }

void AppChild::Poll() {
    static Symbol tool_sync_cam("tool_sync_cam");
    if (mStream) {
        while (mEnabled && !mSync) {
            DataArrayPtr cmd;
            *mStream >> cmd;
            cmd->Execute(true);
        }
        mSync = false;
    }
}
