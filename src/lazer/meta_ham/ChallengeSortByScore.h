#pragma once
#include "ChallengeSort.h"
#include "NavListNode.h"
#include "meta_ham/NavListNode.h"

class ChallengeScoreCmp : public NavListItemSortCmp {
public:
    int Compare(const NavListItemSortCmp *, NavListNodeType) const;

    ChallengeScoreCmp(int type, int i, char const *name)
        : mChallengeSongType(type), mScore(i), mSongName(name) {}

    int mChallengeSongType; // 0x4
    int mScore; // 0x8
    char const *mSongName; // 0xc
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
