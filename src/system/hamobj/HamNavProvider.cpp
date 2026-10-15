#include "HamNavProvider.h"
#include "hamobj/HamNavList.h"
#include "hamobj/HamNavProvider.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "ui/UIListSubList.h"
#include "utl/BinStream.h"
#include "utl/Symbol.h"

HamNavProvider::HamNavProvider() : mNavList(0) {}

HamNavProvider::~HamNavProvider() {
    for (int i = 0; i < mNavItems.size(); i++) {
        if (mNavItems[i].mLabelProvider) {
            RELEASE(mNavItems[i].mLabelProvider);
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
    SYNC_PROP_SET(
        checkbox,
        (int &)o.mCheckboxState,
        o.mCheckboxState = (HamNavProvider::CheckboxMode)_val.Int()
    )
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(HamNavProvider)
    SYNC_PROP_MODIFY(nav_items, mNavItems, if (mNavList) mNavList->Refresh())
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const HamNavProvider::NavItem &item) {
    bs << item.mLabel;
    bs << item.mCheckboxState;
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
        bs >> (int &)item.mCheckboxState;
    }
    if (bs.rev >= 2) {
        bs >> item.mLabels;
    }
    return bs;
}

INIT_REVS(2, 0)

BEGIN_LOADS(HamNavProvider)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mNavItems;
    for (int i = 0; i < mNavItems.size(); i++) {
        mNavItems[i].mLabelProvider = nullptr;
    }
    if (mNavList)
        mNavList->Refresh();
END_LOADS

void HamNavProvider::Text(int showing, int data, UIListLabel *slot, UILabel *label) const {
    if (slot->Matches("label")) {
        if (mNavItems[data].mFmtArgs) {
            mNavItems[data].mFmtArgs->Node(0) = mNavItems[data].mLabel;
            Message msg("set_token_fmt", mNavItems[data].mFmtArgs);
            label->Handle(msg, false);
        } else {
            label->SetTextToken(mNavItems[data].mLabel);
        }
    } else if (slot->Matches("checkbox")) {
        switch (mNavItems[data].mCheckboxState) {
        case 0:
            label->SetIcon('\0');
            break;
        case 1:
            label->SetIcon('a');
            break;
        case 2:
            label->SetIcon('b');
            break;
        default:
            break;
        }
    } else if (slot->Matches("song")) {
        if (mNavItems[data].mFmtArgs) {
            mNavItems[data].mFmtArgs->Node(0) = mNavItems[data].mLabel;
            Message msg("set_token_fmt", mNavItems[data].mFmtArgs);
            label->Handle(msg, false);
        } else {
            label->SetTextToken(mNavItems[data].mLabel);
        }
    } else {
        label->SetTextToken(gNullStr);
    }
}

UIListProvider *HamNavProvider::Provider(int showing, int data, UIListSubList *) const {
    MILO_ASSERT(data < mNavItems.size(), 0x81);
    return mNavItems[data].mLabelProvider;
}

Symbol HamNavProvider::DataSymbol(int idx) const {
    if (idx >= 0 && idx < mNavItems.size()) {
        MILO_ASSERT((0) <= (idx) && (idx) < (mNavItems.size()), 99);
        for (int i = 0; i <= idx; i++) {
            if (mNavItems[i].mHidden)
                idx++;
        }
        return mNavItems[idx].mLabel;
    } else {
        MILO_NOTIFY(
            "HamNavProvider::DataSymbol out of bounds %d %d", idx, mNavItems.size()
        );
        return gNullStr;
    }
}

bool HamNavProvider::IsActive(int idx) const {
    if (idx >= 0 && idx < mNavItems.size()) {
        return mNavItems[idx].mEnabled;
    } else
        return true;
}

bool HamNavProvider::IsHidden(int idx) const {
    if (idx >= 0 && idx < mNavItems.size()) {
        return mNavItems[idx].mHidden;
    } else
        return false;
}

void HamNavProvider::Init() { REGISTER_OBJ_FACTORY(HamNavProvider); }

void HamNavProvider::CreateSubListProvider(int index) {
    NavItem &curItem = mNavItems[index];
    if (!curItem.mLabelProvider) {
        int numLabels = curItem.mLabels.size();
        DataArray *arr = new DataArray(numLabels);
        for (int i = 0; i < numLabels; i++) {
            arr->Node(i) = curItem.mLabels[i];
        }
        curItem.mLabelProvider = new DataProvider(arr, 0, false, false, nullptr);
        arr->Release();
    }
}

Symbol HamNavProvider::DataSymbol(int idx, int subIdx) const {
    MILO_ASSERT((0) <= (idx) && (idx) < (mNavItems.size()), 0x73);
    MILO_ASSERT((0) <= (subIdx) && (subIdx) < (mNavItems[idx].mLabels.size()), 0x74);
    return mNavItems[idx].mLabels[subIdx];
}

int HamNavProvider::FindLabel(Symbol s) {
    for (int i = 0; i < mNavItems.size(); i++) {
        if (s == mNavItems[i].mLabel) {
            return i;
        }
    }
    MILO_ASSERT(false, 0xA9);
    return -1;
}

void HamNavProvider::SetLabel(int elementIndex, int labelIndex, Symbol s) {
    MILO_ASSERT(elementIndex >= 0 && elementIndex < mNavItems.size(), 0xEF);
    NavItem &curItem = mNavItems[elementIndex];
    curItem.mLabels.clear();
    curItem.mLabels.push_back(s);
    if (curItem.mLabelProvider) {
        if (labelIndex < curItem.mLabelProvider->Data()->Size()) {
            DataProvider *prov = curItem.mLabelProvider;
            DataArray *provData = prov->Data();
            DataArray *cloned = prov->Data()->Clone(true, false, 0);
            cloned->Node(labelIndex) = s;
            curItem.mLabelProvider->SetData(cloned);
            cloned->Release();
        }
    } else {
        CreateSubListProvider(elementIndex);
    }
}

void HamNavProvider::SetLabels(int index, DataArray *a) {
    MILO_ASSERT(index >= 0 && index < mNavItems.size(), 0x107);
    NavItem &curItem = mNavItems[index];
    curItem.mLabels.clear();
    for (int i = 0; i < a->Size(); i++) {
        curItem.mLabels.push_back(a->Sym(i));
    }
    if (curItem.mLabelProvider) {
        curItem.mLabelProvider->SetData(a);
    } else {
        CreateSubListProvider(index);
    }
}

void HamNavProvider::SetChecked(Symbol s, bool b2, bool b3) {
    int index = FindLabel(s);
    MILO_ASSERT(mNavItems[index].mCheckboxState != kCheckbox_None, 0xB0);
    if (b2) {
        mNavItems[index].mCheckboxState = kCheckbox_Enabled;
    } else {
        mNavItems[index].mCheckboxState = kCheckbox_Disabled;
    }
    if (b3 && mNavList) {
        mNavList->Refresh();
    }
}

void HamNavProvider::SelectRadioButton(Symbol s) {
    for (int i = 0; i < mNavItems.size(); i++) {
        if (s == mNavItems[i].mLabel) {
            mNavItems[i].mCheckboxState = kCheckbox_Enabled;
        } else {
            mNavItems[i].mCheckboxState = kCheckbox_None;
        }
    }
    if (mNavList) {
        mNavList->Refresh();
    }
}

void HamNavProvider::SetStars(Symbol s, int songID, bool b3) {
    int index = FindLabel(s);
    mNavItems[index].mSongID = songID;
    if (b3) {
        mNavItems[index].mScoreDisplayMode = kScoreDisplay_Multiplayer;
    } else {
        mNavItems[index].mScoreDisplayMode = kScoreDisplay_Perform;
    }
    if (mNavList) {
        mNavList->Refresh();
    }
}

void HamNavProvider::ResetLabelProvider(int idx) {
    NavItem &curItem = mNavItems[idx];
    if (curItem.mLabelProvider) {
        RELEASE(curItem.mLabelProvider);
    }
}

void HamNavProvider::SetLabel(int index, Symbol label) {
    MILO_ASSERT(index >= 0 && index < mNavItems.size(), 0xE8);
    mNavItems[index].mLabel = label;
    if (mNavList) {
        mNavList->Refresh();
    }
}

void HamNavProvider::SetEnabled(int index, bool enabled) {
    MILO_ASSERT(index >= 0 && index < mNavItems.size(), 0x123);
    mNavItems[index].mEnabled = enabled;
    if (!enabled) {
        if (mNavItems[index].mLabelProvider) {
            DataArray *data = mNavItems[index].mLabelProvider->Data();
            for (int i = 0; i < data->Size(); i++) {
                mNavItems[index].mLabelProvider->Disable(data->Node(i).Sym());
            }
        }
    }
    if (mNavList) {
        mNavList->Refresh();
    }
}

bool HamNavProvider::IsEnabled(int index) const {
    MILO_ASSERT(index >= 0 && index < mNavItems.size(), 0x137);
    return mNavItems[index].mEnabled;
}

void HamNavProvider::SetHidden(int index, bool enabled) {
    MILO_ASSERT(index >= 0 && index < mNavItems.size(), 0x14D);
    mNavItems[index].mHidden = enabled;
    if (mNavList) {
        mNavList->Refresh();
    }
}

DataNode HamNavProvider::OnSetEnabled(const DataArray *a) {
    const DataNode &node = a->Evaluate(2);
    if (node.Type() == kDataInt) {
        SetEnabled(node.Int(), a->Int(3));
    } else {
        SetEnabled(FindLabel(node.ForceSym()), a->Int(3));
    }
    return 0;
}

DataNode HamNavProvider::OnSetHidden(const DataArray *a) {
    const DataNode &node = a->Evaluate(2);
    if (node.Type() == kDataInt) {
        SetHidden(node.Int(), a->Int(3));
    } else {
        SetHidden(FindLabel(node.ForceSym()), a->Int(3));
    }
    return 0;
}

DataNode HamNavProvider::OnSetFormatArgs(const DataArray *a) {
    int index = 0;
    const DataNode &node = a->Evaluate(2);
    if (node.Type() == kDataInt) {
        index = node.Int();
    } else {
        index = FindLabel(node.ForceSym());
    }

    MILO_ASSERT(index >= 0 && index < mNavItems.size(), 0x16d);

    if (mNavItems[index].mFmtArgs) {
        mNavItems[index].mFmtArgs->Release();
        mNavItems[index].mFmtArgs = nullptr;
    }

    if (a->Size() > 3) {
        mNavItems[index].mFmtArgs = new DataArray(a->Size() - 2);
        mNavItems[index].mFmtArgs->Node(0) = mNavItems[index].mLabel;

        for (int i = 3; i < a->Size(); i++) {
            mNavItems[index].mFmtArgs->Node(i - 2) = a->Node(i).Evaluate();
        }
    }

    if (mNavList) {
        mNavList->Refresh();
    }
    return 0;
}
