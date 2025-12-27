#pragma once
#include "ui/UIScreen.h"

class HamScreen : public UIScreen {
public:
    HamScreen() {};
    ~HamScreen() {};
    virtual void Exit(UIScreen *);
    virtual bool Exiting() const;
    virtual void Enter(UIScreen *);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool InComponentSelect() const;

protected:
    bool IsEventDialogOnTop() const;
    DataNode OnEventMsgCommon(Message const&);
};

