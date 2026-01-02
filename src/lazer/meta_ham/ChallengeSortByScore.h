#pragma once
#include "ChallengeSort.h"
#include "NavListNode.h"

class ChallengeScoreCmp : public NavListItemSortCmp {
public:
    int Compare(const NavListItemSortCmp *, NavListNodeType) const;
};

class ChallengeSortByScore : public ChallengeSort {

protected:
};
