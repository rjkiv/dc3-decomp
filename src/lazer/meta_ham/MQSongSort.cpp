#include "MQSongSort.h"

#include "AppLabel.h"
#include "ChallengeSort.h"
#include "MQSongSortMgr.h"
#include "MQSongSortNode.h"
#include "meta_ham/NavListNode.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

MQSongSort::MQSongSort() {};

void MQSongSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    TheMQSongSortMgr->ClearHeaders();
}

void MQSongSort::SetHighlightedIx(int i1) {
    mPrevHighlightNode = mHighlightNode;
    if (i1 >= 0) {
        if (mList.size() >= i1) {
            if (mList.size() == 0) {
                return;
            }
            mHighlightNode = mList[i1];
            TheMQSongSortMgr->OnHighlightChanged();
            return;
        }
    }
    mHighlightNode = 0;
}

void MQSongSort::UpdateHighlight() {
    if (mList.size() != 0) {
        NavListSort::UpdateHighlight();
        TheMQSongSortMgr->OnHighlightChanged();
    }
}

void MQSongSort::OnSelectShortcut(int i1) {
    if (mList.size() != 0) {
        NavListSort::OnSelectShortcut(i1);
        TheMQSongSortMgr->OnHighlightChanged();
    }
}

void MQSongSort::Text(int i1, int i2, UIListLabel *listlabel, UILabel *label) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0xd0);

    app_label->SetFromGeneralSelectNode(mTree[i2]);
}

void MQSongSort::SetHighlightItem(const NavListSortNode *node) {
    NavListSortNode *tempNode = mHighlightNode;
    mHighlightNode = nullptr;
    mPrevHighlightNode = tempNode;
    if (node) {
        if (node->GetType() == 5 || node->GetType() == 4) {
            auto find = std::find_if(mList.begin(), mList.end(), SortNodeFind(node));
            if (find != mList.end()) {
                mHighlightNode = *find;
                TheMQSongSortMgr->OnHighlightChanged();
            }
        }
    }
}

void MQSongSort::BuildItemList() {
    Symbol sym(gNullStr);
    auto sortNode = mHighlightNode;
    if (sortNode && sortNode->GetType() == 5) {
        sym = sortNode->GetToken();
    }
    DeleteItemList();
    FOREACH (it, mFunctions) {
        (*it)->Renumber(mList);
    }
    FOREACH (it, mTree) {
        (*it)->Renumber(mList);
    }
    FOREACH (it, mTree) {
        (*it)->FinishBuildList(this);
    }
    if (!sym.Null()) {
        mHighlightNode = GetNode(sym);
    }
    TheMQSongSortMgr->FinalizeHeaders();
}

void MQSongSort::BuildTree() {
    NavListSort::DeleteTree();
    Init();
    std::vector<NavListItemNode *> nodes;

    auto &map = TheMQSongSortMgr->m_songsPerCharacter;
    FOREACH (it, map) {
        FOREACH (it2, it->second) {
            MQSongSortNode *node = static_cast<MQSongSortNode *>(NewItemNode(it2));
            node->mCharacter = it->first;
            nodes.push_back(node);
        }
    }

    auto begin = nodes.begin();
    auto end = nodes.end();
    while (begin != end) {
        std::vector<NavListItemNode *>::iterator it = begin;
        while (it != end) {
            if (static_cast<MQSongSortNode *>(*it)->mCharacter
                != static_cast<MQSongSortNode *>(*begin)->mCharacter) {
                break;
            }
            it++;
        }
        NavListShortcutNode *shortcutNode = NewShortcutNode(*begin);
        mTree.push_back(shortcutNode);
        shortcutNode->InsertHeaderRange(begin, it, this);
        begin = it;
    }

    FOREACH (it, mTree) {
        (*it)->FinishSort(this);
    }
}
