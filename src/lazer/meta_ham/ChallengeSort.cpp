#include "ChallengeSort.h"
#include "AppLabel.h"
#include "ChallengeRecord.h"
#include "ChallengeSortByScore.h"
#include "ChallengeSortMgr.h"
#include "ChallengeSortNode.h"
#include "meta_ham/ChallengeSortByScore.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/NavListSort.h"
#include "obj/Object.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

SortNodeFind::SortNodeFind(const NavListSortNode *node)
    : mSym(node->GetToken()), mType(node->GetType()) {}

bool SortNodeFind::operator()(const NavListSortNode *node) const {
    return node->GetToken() == mSym && node->GetType() == mType;
}

#pragma region ChallengeSort

ChallengeSort::ChallengeSort() {}

BEGIN_HANDLERS(ChallengeSort)
    HANDLE_SUPERCLASS(NavListSort)
END_HANDLERS

void ChallengeSort::SetHighlightedIx(int idx) {
    mPrevHighlightNode = mHighlightNode;
    if (idx >= 0) {
        if (mList.size() >= idx) {
            if (mList.size() == 0)
                return;
            mHighlightNode = mList[idx];
            TheChallengeSortMgr->OnHighlightChanged();
            return;
        }
    }
    mHighlightNode = nullptr;
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
    app_label->SetFromGeneralSelectNode(mTree[i2]);
}

void ChallengeSort::SetHighlightItem(const NavListSortNode *node) {
    mPrevHighlightNode = mHighlightNode;
    mHighlightNode = nullptr;
    if (node) {
        if (node->GetType() == 5 || node->GetType() == 4) {
            auto findNode = std::find_if(mList.begin(), mList.end(), SortNodeFind(node));
            if (findNode != mList.end()) {
                mHighlightNode = *findNode;
                TheChallengeSortMgr->OnHighlightChanged();
            }
        }
    }
}

void ChallengeSort::BuildItemList() {
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
    TheChallengeSortMgr->FinalizeHeaders();
}

void ChallengeSort::BuildTree() {
    NavListSort::DeleteTree();
    Init();
    std::vector<NavListItemNode *> nodes;

    std::vector<ChallengeRecord> &records = TheChallengeSortMgr->GetChallengeRecords();
    for (int i = 0; i < records.size(); i++) {
        ChallengeRecord *record = &records[i];
        if (record->GetChallengeType() != 1) {
            NavListItemNode *node = NewItemNode(record);
            auto bound =
                std::lower_bound(nodes.begin(), nodes.end(), node, CompareHeaders());
            nodes.insert(bound, 1, node);
        }
    }

    static Symbol global_challenge("global_challenge");
    static Symbol dlc_challenge("dlc_challenge");
    String globalChallengeSongName = TheChallenges->GetGlobalChallengeSongName();
    String dlcChallengeSongName = TheChallenges->GetDlcChallengeSongName();

    NavListShortcutNode *globalShortcutNode = new NavListShortcutNode(
        new ChallengeScoreCmp(0, 0, globalChallengeSongName.c_str()),
        global_challenge,
        true
    );

    NavListShortcutNode *dlcShortcutNode = new NavListShortcutNode(
        new ChallengeScoreCmp(1, 0, dlcChallengeSongName.c_str()), dlc_challenge, true
    );

    ChallengeHeaderNode *globalHeaderNode = new ChallengeHeaderNode(
        new ChallengeScoreCmp(0, 0, globalChallengeSongName.c_str()),
        global_challenge,
        true
    );

    ChallengeHeaderNode *dlcHeaderNode = new ChallengeHeaderNode(
        new ChallengeScoreCmp(1, 0, dlcChallengeSongName.c_str()), dlc_challenge, true
    );

    globalShortcutNode->InsertNode(globalHeaderNode);
    dlcShortcutNode->InsertNode(dlcHeaderNode);
    mTree.push_back(globalShortcutNode);
    mTree.push_back(dlcShortcutNode);

    FOREACH (it, nodes) {
        NavListShortcutNode *shortcutNode = NewShortcutNode(*it);
        auto findIf =
            std::find_if(mTree.begin(), mTree.end(), NodeFind(shortcutNode->GetToken()));
        if (findIf == mTree.end()) {
            mTree.push_back(shortcutNode);
        } else {
            delete shortcutNode;
            shortcutNode = *findIf;
        }
        shortcutNode->Insert(*it, this);
    }

    FOREACH (it, mTree) {
        (*it)->FinishSort(this);
    }
}

#pragma endregion
