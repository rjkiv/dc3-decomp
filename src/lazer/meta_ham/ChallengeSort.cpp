#include "ChallengeSort.h"
#include "AppLabel.h"
#include "ChallengeSortMgr.h"
#include "meta_ham/NavListSort.h"
#include "obj/Object.h"

SortNodeFind::SortNodeFind(const NavListSortNode *node)
    : mToken(node->GetToken()), mType(node->GetType()) {}

bool SortNodeFind::operator()(const NavListSortNode *node) const {
    return node->GetToken() == mToken && node->GetType() == mType;
}

#pragma region ChallengeSort

ChallengeSort::ChallengeSort() {}

BEGIN_HANDLERS(ChallengeSort)
    HANDLE_SUPERCLASS(NavListSort)
END_HANDLERS

void ChallengeSort::SetHighlightedIx(int idx) {
    unk54 = unk50;
    if (idx >= 0) {
        if (mList.size() >= idx) {
            if (mList.empty())
                return;
            unk50 = mList[idx];
            TheChallengeSortMgr->OnHighlightChanged();
            return;
        }
    }
    unk50 = nullptr;
}

void ChallengeSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    TheChallengeSortMgr->ClearHeaders();
}

void ChallengeSort::UpdateHighlight() {
    if (mList.size() != 0) {
        NavListSort::UpdateHighlight();
        TheChallengeSortMgr->OnHighlightChanged();
    }
}

void ChallengeSort::OnSelectShortcut(int idx) {
    if (mList.size() != 0) {
        NavListSort::OnSelectShortcut(idx);
        TheChallengeSortMgr->OnHighlightChanged();
    }
}

void ChallengeSort::Text(int i1, int i2, UIListLabel *listlabel, UILabel *label) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0xe1);
    app_label->SetFromGeneralSelectNode(unk30[i2]);
}

void ChallengeSort::SetHighlightItem(const NavListSortNode *node) {
    unk54 = unk50;
    unk50 = nullptr;
    if (node) {
        if (node->GetType() == 5 || node->GetType() == 4) {
            auto findNode = std::find_if(mList.begin(), mList.end(), SortNodeFind(node));
            if (findNode != mList.end()) {
                unk50 = *findNode;
                TheChallengeSortMgr->OnHighlightChanged();
            }
        }
    }
}

void ChallengeSort::BuildItemList() {
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
    TheChallengeSortMgr->FinalizeHeaders();
}

#pragma endregion
