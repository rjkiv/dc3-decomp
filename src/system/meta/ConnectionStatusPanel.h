#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

DECLARE_MESSAGE(ConnectionStatusChangedMsg, "connection_status_changed")
ConnectionStatusChangedMsg(int);
bool Connected() const { return mData->Int(2); }
END_MESSAGE

class ConnectionStatusPanel : public UIPanel {
public:
    // Hmx::Object
    OBJ_CLASSNAME(ConnectionStatusPanel)
    OBJ_SET_TYPE(ConnectionStatusPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Enter();
    virtual void Exit();

    ConnectionStatusPanel();
    ~ConnectionStatusPanel();

protected:
    void CheckForLostConnection();
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
};
