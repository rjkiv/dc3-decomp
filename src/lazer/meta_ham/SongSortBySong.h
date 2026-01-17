#pragma once
#include "NavListNode.h"
#include "SongSort.h"

class SongCmp : public NavListItemSortCmp {
public:
    SongCmp(const char *c1, const char *c2) : unk4(c1), unk8(c2) {};

    virtual ~SongCmp();

    virtual int Compare(const NavListItemSortCmp *, NavListNodeType) const;

    const char *unk4;
    const char *unk8;
};

class SongSortBySong : public SongSort {
public:
    SongSortBySong();
    virtual ~SongSortBySong();

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;
};
