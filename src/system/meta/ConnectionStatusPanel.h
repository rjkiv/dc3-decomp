#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"

DECLARE_MESSAGE(ConnectionStatusChangedMsg, "connection_status_changed")
ConnectionStatusChangedMsg(bool b1) : Message(Type(), b1) {}
bool Connected() const { return mData->Int(2); }
END_MESSAGE

class ConnectionStatusPanel : public UIPanel {
public:
    ConnectionStatusPanel();
    // Hmx::Object
    virtual ~ConnectionStatusPanel();
    OBJ_CLASSNAME(ConnectionStatusPanel)
    OBJ_SET_TYPE(ConnectionStatusPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Enter();
    virtual void Exit();

    NEW_OBJ(ConnectionStatusPanel)

protected:
    void CheckForLostConnection();
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
};
