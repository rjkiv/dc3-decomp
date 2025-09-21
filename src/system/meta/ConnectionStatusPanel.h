#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"


DECLARE_MESSAGE(ConnectionStatusChangedMsg, "connection_status_changed")
ConnectionStatusChangedMsg(int);
bool Connected() const { return mData->Int(2); }
END_MESSAGE

class ConnectionStatusPanel : public UIPanel{
    public:
        ConnectionStatusPanel();
        OBJ_CLASSNAME(ConnectionStatusPanel);
        virtual void Exit();
        OBJ_SET_TYPE(ConnectionStatusPanel)
        ~ConnectionStatusPanel();
        virtual void Enter();
        virtual DataNode Handle(DataArray *, bool);
    protected:
        void CheckForLostConnection();
        DataNode OnMsg(const ConnectionStatusChangedMsg &);
};
