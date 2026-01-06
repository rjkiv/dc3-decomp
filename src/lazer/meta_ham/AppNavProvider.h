#pragma once
#include "hamobj/HamNavProvider.h"
#include "ui/UIListLabel.h"
#include "obj/Object.h"

class AppNavProvider : public HamNavProvider {
public:
    AppNavProvider() {}
    ~AppNavProvider();
    OBJ_CLASSNAME(HamNavProvider); // bruh
    OBJ_SET_TYPE(AppNavProvider);
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual void Custom(int, int, UIListCustom *, Hmx::Object *) const;
    virtual DataNode Handle(DataArray *, bool);

    NEW_OBJ(AppNavProvider)
};
