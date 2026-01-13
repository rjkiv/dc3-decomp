#pragma once
#include "NavListNode.h"
#include "SongSort.h"
#include "SongSortNode.h"

class DifficultyCmp : public NavListItemSortCmp {
public:
    DifficultyCmp(int tier, float rank, const char *name) {
        mTier = tier;
        mRank = rank;
        mName = name;
    };
    virtual ~DifficultyCmp();

    virtual int Compare(const NavListItemSortCmp *,  NavListNodeType) const;

    int mTier; // 0x0
    float mRank; // 0x4
    const char *mName; // 0x8
};

class SongSortByDiff : public SongSort {
public:
    SongSortByDiff();
    virtual ~SongSortByDiff() {};

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
};
