#include "MQSongSort.h"

#include "AppLabel.h"
#include "ChallengeSort.h"
#include "MQSongSortMgr.h"
#include "MQSongSortNode.h"
#include "meta_ham/NavListNode.h"
#include "stl/_algo.h"
#include "stl/_vector.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

MQSongSort::MQSongSort() {};

void MQSongSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    TheMQSongSortMgr->ClearHeaders();
}

void MQSongSort::SetHighlightedIx(int i1) {
    unk54 = unk50;
    if (i1 >= 0) {
        if (mList.size() >= i1) {
            if (mList.size() == 0) {
                return;
            }
            unk50 = mList[i1];
            TheMQSongSortMgr->OnHighlightChanged();
            return;
        }
    }
    unk50 = 0;
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

    app_label->SetFromGeneralSelectNode(unk30[i2]);
}

void MQSongSort::SetHighlightItem(const NavListSortNode *node) {
    NavListSortNode *tempNode = unk50;
    unk50 = nullptr;
    unk54 = tempNode;
    if (node) {
        if (node->GetType() == 5 || node->GetType() == 4) {
            auto find = std::find_if(mList.begin(), mList.end(), SortNodeFind(node));
            if (find != mList.end()) {
                unk50 = *find;
                TheMQSongSortMgr->OnHighlightChanged();
            }
        }
    }
}

void MQSongSort::BuildItemList() {
    Symbol sym(gNullStr);
    auto sortNode = unk50;
    if (sortNode && sortNode->GetType() == 5) {
        sym = sortNode->GetToken();
    }
    DeleteItemList();
    FOREACH (it, unk3c) {
        (*it)->Renumber(mList);
    }
    FOREACH (it, unk30) {
        (*it)->Renumber(mList);
    }
    FOREACH (it, unk30) {
        (*it)->FinishBuildList(this);
    }
    if (!sym.Null()) {
        unk50 = GetNode(sym);
    }
    TheMQSongSortMgr->FinalizeHeaders();
}

void MQSongSort::BuildTree() {
    NavListSort::DeleteTree();
    Init();
    std::vector<NavListItemNode *> nodes;

    auto &map = TheMQSongSortMgr->GetUnk78();
    for (auto it = map.begin(); it != map.end(); ++it) {
        FOREACH (it2, it->second) {
            NavListItemNode *node = NewItemNode(it2);
            static_cast<MQSongSortNode *>(node)->SetUnk4C(it->first);
            nodes.push_back(node);
        }
    }

    auto begin = nodes.begin();
    auto end = nodes.end();
    while (begin != end) {
        std::vector<NavListItemNode *>::iterator it = begin;
        while (it != end) {
            if (static_cast<MQSongSortNode *>(*it)->GetUnk4C()
                != static_cast<MQSongSortNode *>(*begin)->GetUnk4C()) {
                break;
            }
            it++;
        }
        NavListShortcutNode *shortcutNode = NewShortcutNode(*begin);
        unk30.push_back(shortcutNode);
        shortcutNode->InsertHeaderRange(begin, it, this);
        begin = it;
    }

    FOREACH (it, unk30) {
        (*it)->FinishSort(this);
    }
}
