#pragma once
#include "ChallengeSort.h"
#include "NavListNode.h"
#include "meta_ham/NavListNode.h"

class ChallengeScoreCmp : public NavListItemSortCmp {
public:
    int Compare(const NavListItemSortCmp *, NavListNodeType) const;
};

class ChallengeSortByScore : public ChallengeSort {
public:
    ChallengeSortByScore() {
        static Symbol by_score("by_score");
        mSortName = by_score;
    }
    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;
};
