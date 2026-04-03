#include "meta_ham/NavListSort.h"
#include "math/Utl.h"
#include "meta_ham/NavListNode.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UI.h"
#include "utl/Locale.h"

struct NodeFind {
    NodeFind(Symbol t) : token(t) {}
    bool operator()(const NavListNode *n) const { return n->GetToken() == token; }

    Symbol token;
};

NavListSort::NavListSort() : unk30(0), unk50(0), unk54(0) {}

BEGIN_HANDLERS(NavListSort)
    HANDLE_EXPR(get_current_shortcut, GetCurrentShortcut())
    HANDLE_ACTION(select_shortcut, OnSelectShortcut(_msg->Int(2)))
    HANDLE_ACTION(highlight_next_header, ChangeHighlightHeader(1))
    HANDLE_ACTION(highlight_prev_header, ChangeHighlightHeader(-1))
    HANDLE_EXPR(sort_name, mSortName)
    HANDLE_EXPR(highlight_shortcut_str, HighlightTokenStr())
END_HANDLERS

void NavListSort::DeleteItemList() {
    mList.clear();
    FOREACH (it, unk3c) {
        (*it)->DeleteAll();
        RELEASE(*it);
    }
    unk3c.clear();
}

void NavListSort::UpdateHighlight() {
    MILO_ASSERT(mList.size() && mList[0], 0xE1);
    int i6 = -1;
    unk54 = unk50;
    if (!unk50) {
        unk50 = mList[0];
    } else {
        i6 = unk50->StartIndex();
    }
    int idx = unk50->StartIndex();
    while (!unk50->IsEnabled()) {
        idx = (idx + 1) % mList.size();
        if (i6 == idx)
            break;
        unk50 = mList[idx];
    }
}

void NavListSort::OnSelectShortcut(int idx) {
    unk50 = unk30[idx]->GetFirstActive();
    static Symbol skip_to_ix("skip_to_ix");
    static Message msg(skip_to_ix, 0);
    msg[0] = unk50->StartIndex();
    TheUI->Handle(msg, false);
}

const char *NavListSort::HighlightTokenStr() const {
    NavListShortcutNode *shortcut = unk50->GetShortcut();
    if (shortcut->LocalizeToken()) {
        return Localize(shortcut->GetToken(), nullptr, TheLocale);
    } else {
        return shortcut->GetToken().Str();
    }
}

NavListSortNode *NavListSort::GetNode(Symbol s) const {
    FOREACH (it, mList) {
        if (s == (*it)->GetToken())
            return *it;
    }
    return nullptr;
}

int NavListSort::GetDataCount() const { return mList.size(); }

void NavListSort::DeleteTree() {
    unk54 = 0;
    unk50 = 0;
    mList.clear();
    FOREACH (it, unk30) {
        (*it)->DeleteAll();
        RELEASE(*it);
    }
    unk30.clear();
}

bool NavListSort::SetHighlightID(DataArray *a) {
    NavListSortNode *tmp = unk50;
    unk50 = nullptr;
    unk54 = tmp;
    int aSize = a->Size();
    if (aSize == 0)
        return false;
    if (aSize == 1) {
        auto it = std::find_if(unk3c.begin(), unk3c.end(), NodeFind(a->Sym(0)));
        if (it == unk3c.end())
            return false;
        unk50 = *it;
        return true;

    } else {
        auto si = std::find_if(unk30.begin(), unk30.end(), NodeFind(a->Sym(0)));
        if (si == unk30.end())
            return false;
        MILO_ASSERT(kNodeShortcut == (*si)->GetType(), 0x44);
        const std::list<NavListSortNode *> &children = (*si)->Children();
        auto it = std::find_if(children.begin(), children.end(), NodeFind(a->Sym(1)));
        if (it == children.end())
            return false;
        MILO_ASSERT(kNodeHeader == (*it)->GetType(), 0x4E);
        if (aSize == 2) {
            unk50 = *it;
            return true;
        }
        const std::list<NavListSortNode *> &grandChildren = (*it)->Children();
        auto gIt =
            std::find_if(grandChildren.begin(), grandChildren.end(), NodeFind(a->Sym(2)));
        if (gIt == grandChildren.end())
            return false;
        if (aSize == 3) {
            unk50 = *gIt;
            return true;
        }
        const std::list<NavListSortNode *> &greatGrandChildren = (*gIt)->Children();
        auto ggIt = std::find_if(
            greatGrandChildren.begin(), greatGrandChildren.end(), NodeFind(a->Sym(3))
        );
        if (ggIt == greatGrandChildren.end())
            return false;
        else {
            unk50 = *gIt;
            return true;
        }
    }
}

int NavListSort::GetCurrentShortcut() {
    if (!unk50)
        return 0;
    else {
        NavListShortcutNode *shortcut = unk50->GetShortcut();
        for (int i = 0; i < unk30.size(); i++) {
            if (!unk30[i]->Compare(shortcut, kNodeShortcut)) {
                return i;
            }
        }
        MILO_FAIL("Shortcut not found for this entry!\n");
        return -1;
    }
}

// void NavListSort::ChangeHighlightHeader(int dir) {
//     if (dir != 1 && dir != -1)
//         MILO_ASSERT(dir == 1 || dir == -1, 0xa0);
// }
