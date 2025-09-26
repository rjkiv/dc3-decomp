#include "meta/ConnectionStatusPanel.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

ConnectionStatusPanel::ConnectionStatusPanel(){}

ConnectionStatusPanel::~ConnectionStatusPanel(){}

void ConnectionStatusPanel::Exit(){
    static Symbol connection_status_changed("connection_status_changed");
    ThePlatformMgr.RemoveSink(this, connection_status_changed);
    UIPanel::Exit();
}

void ConnectionStatusPanel::Enter(){
    UIPanel::Enter();
    static Symbol connection_status_changed("connection_status_changed");
    ThePlatformMgr.AddSink(this, connection_status_changed);
    CheckForLostConnection();
}

void ConnectionStatusPanel::CheckForLostConnection(){
    if(!ThePlatformMgr.IsConnected()){
        static Message msg("on_connection_lost");
        Handle(msg,true);
    }
}

DataNode ConnectionStatusPanel::OnMsg(const ConnectionStatusChangedMsg &msg){
    CheckForLostConnection();
    return 0;
}

BEGIN_HANDLERS(ConnectionStatusPanel)
    HANDLE_MESSAGE(ConnectionStatusChangedMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS
