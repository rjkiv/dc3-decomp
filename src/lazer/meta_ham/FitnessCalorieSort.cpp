#include "meta_ham/FitnessCalorieSort.h"
#include "FitnessCalorieSortMgr.h"
#include "NavListSort.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/ChallengeSort.h"
#include "meta_ham/NavListNode.h"
#include "os/Debug.h"
#include "rndobj/Mesh.h"
#include "stl/_algo.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

FitnessCalorieSort::FitnessCalorieSort() {}

void FitnessCalorieSort::Text(
    int, int idx, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0xa4);
    app_label->SetFromGeneralSelectNode(unk30[idx]);
}
void FitnessCalorieSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    TheFitnessCalorieSortMgr->ClearHeaders();
}

void FitnessCalorieSort::SetHighlightedIx(int idx) {
    unk54 = unk50;
    if (0 <= idx) {
        if (mList.size() >= idx) {
            if (mList.size() == 0) {
                return;
            }
            unk50 = mList[idx];
            TheFitnessCalorieSortMgr->OnHighlightChanged();
            return;
        }
    }
    unk50 = 0;
}

void FitnessCalorieSort::UpdateHighlight() {
    if (mList.size() != 0) {
        NavListSort::UpdateHighlight();
        TheFitnessCalorieSortMgr->OnHighlightChanged();
    }
}

void FitnessCalorieSort::OnSelectShortcut(int i) {
    if (mList.size() != 0) {
        NavListSort::OnSelectShortcut(i);
        TheFitnessCalorieSortMgr->OnHighlightChanged();
    }
}

void FitnessCalorieSort::SetHighlightItem(NavListSortNode const *node) {
    NavListSortNode *tempNode = unk50;
    unk50 = nullptr;
    unk54 = tempNode;
    if (node) {
        if (node->GetType() == 5 || node->GetType() == 4) {
            auto find = std::find_if(mList.begin(), mList.end(), SortNodeFind(node));
            if (find != mList.end()) {
                unk50 = *find;
                TheFitnessCalorieSortMgr->OnHighlightChanged();
            }
        }
    }
}

void FitnessCalorieSort::BuildItemList() {
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
    TheFitnessCalorieSortMgr->FinalizeHeaders();
}

void FitnessCalorieSort::BuildTree() {
    NavListSort::DeleteTree();
    Init();
    std::vector<NavListItemNode *> nodes;

    std::vector<int> &values = TheFitnessCalorieSortMgr->GetUnk78();
    for (int i = 0; i < values.size(); i++) {
        nodes.push_back(NewItemNode(&values[i]));
    }

    int groupSize = TheFitnessCalorieSortMgr->GetGroupSize();
    auto begin = nodes.begin();
    auto end = nodes.end();
    while (begin != end) {
        std::vector<NavListItemNode *>::iterator it;
        // if not enough nodes to make a set of size "groupsize", go to end
        if (end - begin <= groupSize) {
            it = end;
            // else move forward of size "groupsize" nodes
        } else {
            it = begin + groupSize;
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
