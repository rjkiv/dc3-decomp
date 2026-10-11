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

NavListSort::NavListSort() : mTree(0), mHighlightNode(0), mPrevHighlightNode(0) {}

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
    FOREACH (it, mFunctions) {
        (*it)->DeleteAll();
        RELEASE(*it);
    }
    mFunctions.clear();
}

void NavListSort::UpdateHighlight() {
    MILO_ASSERT(mList.size() && mList[0], 0xE1);
    int i6 = -1;
    mPrevHighlightNode = mHighlightNode;
    if (!mHighlightNode) {
        mHighlightNode = mList[0];
    } else {
        i6 = mHighlightNode->GetIndex();
    }
    int idx = mHighlightNode->GetIndex();
    while (!mHighlightNode->IsActive()) {
        idx = (idx + 1) % mList.size();
        if (i6 == idx)
            break;
        mHighlightNode = mList[idx];
    }
}

void NavListSort::OnSelectShortcut(int idx) {
    mHighlightNode = mTree[idx]->GetFirstActive();
    static Symbol skip_to_ix("skip_to_ix");
    static Message msg(skip_to_ix, 0);
    msg[0] = mHighlightNode->GetIndex();
    TheUI->Handle(msg, false);
}

const char *NavListSort::HighlightTokenStr() const {
    NavListShortcutNode *shortcut = mHighlightNode->GetShortcut();
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
    mPrevHighlightNode = 0;
    mHighlightNode = 0;
    mList.clear();
    FOREACH (it, mTree) {
        (*it)->DeleteAll();
        RELEASE(*it);
    }
    mTree.clear();
}

bool NavListSort::SetHighlightID(DataArray *a) {
    NavListSortNode *tmp = mHighlightNode;
    mHighlightNode = nullptr;
    mPrevHighlightNode = tmp;
    int aSize = a->Size();
    if (aSize == 0) {
        return false;
    }
    if (aSize == 1) {
        Symbol token = a->Sym(0);
        auto it = std::find_if(mFunctions.begin(), mFunctions.end(), NodeFind(token));
        if (it == mFunctions.end())
            return false;
        else {
            mHighlightNode = *it;
            return true;
        }
    } else {
        Symbol token = a->Sym(0);
        auto si = std::find_if(mTree.begin(), mTree.end(), NodeFind(token));
        if (si == mTree.end()) {
            return false;
        }
        MILO_ASSERT(kNodeShortcut == (*si)->GetType(), 0x44);
        std::list<NavListSortNode *> &children = (*si)->GetChildren();
        Symbol token1 = a->Sym(1);
        auto it = std::find_if(children.begin(), children.end(), NodeFind(token1));
        if (it == children.end()) {
            return false;
        }
        MILO_ASSERT(kNodeHeader == (*it)->GetType(), 0x4E);
        if (aSize == 2) {
            mHighlightNode = *it;
            return true;
        }
        std::list<NavListSortNode *> &grandChildren = (*it)->GetChildren();
        Symbol token2 = a->Sym(2);
        auto gIt =
            std::find_if(grandChildren.begin(), grandChildren.end(), NodeFind(token2));
        if (gIt == grandChildren.end()) {
            return false;
        }
        if (aSize == 3) {
            mHighlightNode = *gIt;
            return true;
        }
        std::list<NavListSortNode *> &greatGrandChildren = (*gIt)->GetChildren();
        Symbol token3 = a->Sym(3);
        auto ggIt = std::find_if(
            greatGrandChildren.begin(), greatGrandChildren.end(), NodeFind(token3)
        );
        if (ggIt == greatGrandChildren.end()) {
            return false;
        } else {
            mHighlightNode = *gIt;
            return true;
        }
    }
}

int NavListSort::GetCurrentShortcut() {
    if (!mHighlightNode)
        return 0;
    else {
        NavListShortcutNode *shortcut = mHighlightNode->GetShortcut();
        for (int i = 0; i < mTree.size(); i++) {
            if (!mTree[i]->Compare(shortcut, kNodeShortcut)) {
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
