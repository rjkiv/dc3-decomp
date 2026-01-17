#pragma once
#include "NavListNode.h"
#include "SongSort.h"

int ConvertGameOriginSymbolToEnum(Symbol);

class LocationCmp : public NavListItemSortCmp {
public:
    LocationCmp();
    virtual ~LocationCmp();

    virtual int Compare(const NavListItemSortCmp *, NavListNodeType) const;

    const char *mName; // 0x4
    Symbol mLocation; // 0x8
};

class SongSortByLocation : public SongSort {
public:
    SongSortByLocation();
    virtual ~SongSortByLocation();

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
};
