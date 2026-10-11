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
    app_label->SetFromPlaylistSelectNode(mTree[data]);
}

void PlaylistSort::SetHighlightedIx(int i) {
    mPrevHighlightNode = mHighlightNode;
    if (i >= 0 && static_cast<unsigned int>(GetListSize()) >= i) { // lol
        mHighlightNode = mList[i];
        ThePlaylistSortMgr->OnHighlightChanged();
        return;
    }
    mHighlightNode = nullptr;
}

void PlaylistSort::BuildItemList() {
    Symbol sym(gNullStr);
    NavListSortNode *sortNode = mHighlightNode;
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
    ThePlaylistSortMgr->FinalizeHeaders();
}

void PlaylistSort::BuildTree() {
    DeleteTree();
    Init();
    std::vector<NavListItemNode *> nodes;
    auto &playlists = ThePlaylistSortMgr->GetPlaylists();
    FOREACH (it, playlists) {
        nodes.push_back(NewItemNode(*it));
    }
    auto begin = nodes.begin();
    auto end = nodes.end();
    while (begin != end) {
        auto headerRange =
            std::equal_range(nodes.begin(), nodes.end(), *begin, CompareHeaders());
        NavListShortcutNode *node = NewShortcutNode(*begin);
        mTree.push_back(node);
        begin = headerRange.second;
        node->InsertHeaderRange(headerRange.first, headerRange.second, this);
    }
    FOREACH (it, mTree) {
        (*it)->FinishSort(this);
    }
}

void PlaylistSort::SetHighlightItem(NavListSortNode const *node) {
    NavListSortNode *tempNode = mHighlightNode;
    mHighlightNode = nullptr;
    mPrevHighlightNode = tempNode;
    if (node) {
        if (node->GetType() == 5 || node->GetType() == 4) {
            auto find = std::find_if(mList.begin(), mList.end(), SortNodeFind(node));
            if (find != mList.end()) {
                mHighlightNode = *find;
                ThePlaylistSortMgr->OnHighlightChanged();
            }
        }
    }
}
