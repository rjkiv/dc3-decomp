#include "AppNavProvider.h"

#include "ui/UIListCustom.h"

BEGIN_HANDLERS(AppNavProvider)
    HANDLE_SUPERCLASS(HamNavProvider)
END_HANDLERS

void AppNavProvider::Text(int i1, int i2, UIListLabel *listlabel, UILabel *label) const {
    if (dynamic_cast<UIListLabel *>(listlabel)->Matches("practice_diff")) {

    }
}

void AppNavProvider::Custom(int i1, int i2, UIListCustom *listcustom, Hmx::Object *obj) const {
    if (dynamic_cast<UIListLabel *>(listcustom)->Matches("stars")) {

    }
}