#pragma once
#include "NavListNode.h"
#include "SongSort.h"
#include "SongSortNode.h"

class DifficultyCmp : public NavListItemSortCmp {
    public:
    DifficultyCmp(int, int, const char *);
    virtual ~DifficultyCmp() {}

    virtual int Compare(const NavListItemSortCmp *,  NavListNodeType) const;

    int mTier; // 0x4
    float mRank; // 0x8
    const char *mName; // 0xc
};

class SongSortByDiff : public SongSort {
public:
    SongSortByDiff();
    virtual ~SongSortByDiff() {};

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;

protected:
    const char *unk58; // 0x58 - shows up as 0x5c in asm so we off by 4bytes somewhere
};
