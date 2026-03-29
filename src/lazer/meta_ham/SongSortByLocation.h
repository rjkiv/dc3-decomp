#pragma once
#include "NavListNode.h"
#include "SongSort.h"
#include "utl/Symbol.h"

int ConvertGameOriginSymbolToEnum(Symbol);

class LocationCmp : public NavListItemSortCmp {
public:
    virtual ~LocationCmp();

    virtual int Compare(const NavListItemSortCmp *, NavListNodeType) const;

    LocationCmp(const char *name, Symbol location) : mName(name), mLocation(location) {}

    const char *mName; // 0x4
    Symbol mLocation; // 0x8
};

class SongSortByLocation : public SongSort {
public:
    SongSortByLocation() {
        static Symbol by_location("by_location");
        SetSortName(by_location);
    }
    virtual ~SongSortByLocation();

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
};
