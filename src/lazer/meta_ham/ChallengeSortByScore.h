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
    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;

    ChallengeSortByScore();

protected:
};
