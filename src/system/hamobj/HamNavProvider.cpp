#include "HamNavProvider.h"
#include "hamobj/HamNavList.h"
#include "hamobj/HamNavProvider.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

HamNavProvider::HamNavProvider() : mNavList(0) {}

HamNavProvider::~HamNavProvider() {
    for (int i = 0; i < mNavItems.size(); i++) {
        if (mNavItems[i].unk24) {
            RELEASE(mNavItems[i].unk24);
        }
    }
}

BEGIN_HANDLERS(HamNavProvider)
    HANDLE_ACTION(
        set_checked,
        SetChecked(_msg->ForceSym(2), _msg->Int(3), _msg->Size() > 4 ? _msg->Int(4) : true)
    )
    HANDLE_ACTION(select_radio_button, SelectRadioButton(_msg->Sym(2)))
    HANDLE_ACTION(set_stars, SetStars(_msg->ForceSym(2), _msg->Int(3), _msg->Int(4)))
    HANDLE_ACTION(set_label, SetLabel(_msg->Int(2), _msg->ForceSym(3)))
    HANDLE(set_enabled, OnSetEnabled)
    HANDLE(set_hidden, OnSetHidden)
    HANDLE(set_format_args, OnSetFormatArgs)
    HANDLE_ACTION(append_nav_item, mNavItems.push_back(NavItem()))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(HamNavProvider::NavItem)
    SYNC_PROP(label, o.mLabel)
    SYNC_PROP(labels, o.mLabels)
    SYNC_PROP_SET(checkbox, o.mCheckbox, o.mCheckbox = _val.Int())
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(HamNavProvider)
    SYNC_PROP_MODIFY(nav_items, mNavItems, Refresh())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const HamNavProvider::NavItem &item) {
    bs << item.mLabel;
    bs << item.mCheckbox;
    bs << item.mLabels;
    return bs;
}

BEGIN_SAVES(HamNavProvider)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mNavItems;
END_SAVES

BEGIN_COPYS(HamNavProvider)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(HamNavProvider)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mNavItems)
    END_COPYING_MEMBERS
END_COPYS

BinStreamRev &operator>>(BinStreamRev &bs, HamNavProvider::NavItem &item) {
    bs >> item.mLabel;
    if (bs.rev > 0) {
        bs >> item.mCheckbox;
    }
    if (bs.rev >= 2) {
        bs >> item.mLabels;
    }
    return bs;
}

BEGIN_LOADS(HamNavProvider)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bsrev >> mNavItems;
    for (int i = 0; i < mNavItems.size(); i++) {
        mNavItems[i].unk24 = 0;
    }
    Refresh();
END_LOADS

bool HamNavProvider::IsHidden(int idx) const {
    if (idx >= 0 && idx < mNavItems.size()) {
        return mNavItems[idx].unk11;
    } else
        return false;
}

void HamNavProvider::Init() { REGISTER_OBJ_FACTORY(HamNavProvider); }

void HamNavProvider::Refresh() {
    if (mNavList)
        mNavList->Refresh();
}

void HamNavProvider::CreateSubListProvider(int i1) {
    NavItem &curItem = mNavItems[i1];
    if (!curItem.unk24) {
        int numLabels = curItem.mLabels.size();
        DataArray *arr = new DataArray(numLabels);
        for (int i = 0; i < numLabels; i++) {
            arr->Node(i) = curItem.mLabels[i];
        }
        curItem.unk24 = new DataProvider(arr, 0, false, false, nullptr);
        arr->Release();
    }
}
