#pragma once
#include "NavListNode.h"
#include "SongSort.h"
#include "utl/Symbol.h"

class SongCmp : public NavListItemSortCmp {
public:
    SongCmp(const char *c1, const char *c2) : mName(c1), mOtherName(c2) {};

    virtual ~SongCmp();

    virtual int Compare(const NavListItemSortCmp *, NavListNodeType) const;

    const char *mName; // 0x4
    const char *mOtherName; // 0x8
};

class SongSortBySong : public SongSort {
public:
    SongSortBySong() {
        static Symbol by_song("by_song");
        SetSortName(by_song);
    }
    virtual ~SongSortBySong();

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;
};
