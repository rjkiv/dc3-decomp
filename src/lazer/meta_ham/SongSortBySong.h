#pragma once
#include "NavListNode.h"
#include "SongSort.h"

class SongCmp : public NavListItemSortCmp {
public:
    SongCmp();
    virtual ~SongCmp();

    virtual int Compare(const NavListItemSortCmp *,  NavListNodeType) const;

    const char *unk4;
    const char *unk8;
};

class SongSortBySong : public SongSort {
public:
    SongSortBySong();
    virtual ~SongSortBySong();

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
};
