#pragma once
#include "obj/Object.h"
#include "ui/UIScreen.h"

class HamScreen : public UIScreen {
public:
    // Hmx::Object
    virtual ~HamScreen() {};
    OBJ_CLASSNAME(HamScreen)
    OBJ_SET_TYPE(HamScreen)
    virtual DataNode Handle(DataArray *, bool);

    // UIScreen
    virtual bool InComponentSelect() const;
    virtual void Enter(UIScreen *);
    virtual void Exit(UIScreen *);
    virtual bool Exiting() const;

    NEW_OBJ(HamScreen)

    HamScreen() {};

protected:
    bool IsEventDialogOnTop() const;
    DataNode OnEventMsgCommon(Message const &);
};
