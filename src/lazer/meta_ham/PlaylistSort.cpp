#include "meta_ham/PlaylistSort.h"

#include "ChallengeSort.h"
#include "NavListNode.h"
#include "PlaylistSortMgr.h"
#include "macros.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/NavListSort.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"

PlaylistSort::PlaylistSort() {}

void PlaylistSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    ThePlaylistSortMgr->ClearHeaders();
}

void PlaylistSort::UpdateHighlight() {
    NavListSort::UpdateHighlight();
    ThePlaylistSortMgr->OnHighlightChanged();
}

void PlaylistSort::OnSelectShortcut(int i) {
    NavListSort::OnSelectShortcut(i);
    ThePlaylistSortMgr->OnHighlightChanged();
}

void PlaylistSort::Text(int, int data, UIListLabel *uiListLabel, UILabel *uiLabel) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x96);
    app_label->SetFromPlaylistSelectNode(unk30[data]);
}

void PlaylistSort::SetHighlightedIx(int i) {
    unk54 = unk50;
    if (i >= 0 && static_cast<unsigned int>(GetListSize()) >= i) { //lol
        unk50 = mList[i];
        ThePlaylistSortMgr->OnHighlightChanged();
        return;
    }
    unk50 = nullptr;
}

void PlaylistSort::BuildItemList() {
    Symbol sym(gNullStr);
    auto sortNode = unk50;
    if (sortNode && sortNode->GetType() == 5) {
        sym = sortNode->GetToken();
    }
    DeleteItemList();
    FOREACH(it, unk3c) {
        (*it)->Renumber(mList);
    }
    FOREACH(it, unk30) {
        (*it)->Renumber(mList);
    }
    FOREACH(it, unk30) {
        (*it)->FinishBuildList(this);
    }
    if (!sym.Null()) {
        unk50 = GetNode(sym);
    }
    ThePlaylistSortMgr->FinalizeHeaders();
}

void PlaylistSort::BuildTree() {
    DeleteTree();
    Init();
    std::vector<NavListItemNode *> nodes;
    FOREACH(it, ThePlaylistSortMgr->unk78) {
        nodes.push_back(NewItemNode(it));
    }
    FOREACH(it, nodes) {
        CompareHeaders cmpHeaders;
        auto headerRange = std::equal_range(nodes.begin(), nodes.end(), *it,  cmpHeaders);
        NavListShortcutNode *node = NewShortcutNode(*it);
        unk30.push_back(node);
        node->InsertHeaderRange(headerRange.first, headerRange.second, this);
    }
    FOREACH(it, unk30) {
        (*it)->FinishSort(this);
    }
}

void PlaylistSort::SetHighlightItem(NavListSortNode const *node) {
    NavListSortNode *tempNode = unk50;
    unk50 = nullptr;
    unk54 = tempNode;
    if (node) {
        if (node->GetType() == 5 || node->GetType() == 4) {
            auto find = std::find_if(mList.begin(), mList.end(), SortNodeFind(node));
            if (find != mList.end()) {
                unk50 = *find;
                ThePlaylistSortMgr->OnHighlightChanged();
            }
        }
    }
}