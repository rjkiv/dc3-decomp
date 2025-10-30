#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "ui/UI.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"
class SigninScreen : public UIScreen {
public:
    // UIScreen
    OBJ_CLASSNAME(SigninScreen);
    OBJ_SET_TYPE(SigninScreen);
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();
    virtual void Enter(UIScreen *);
    virtual void Exit(UIScreen *);

    SigninScreen();

protected:
    DataNode OnMsg(SigninChangedMsg const &);
    DataNode OnMsg(UIChangedMsg const &);
};
