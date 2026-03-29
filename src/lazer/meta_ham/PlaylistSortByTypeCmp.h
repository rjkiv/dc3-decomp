#pragma once

#include "meta_ham/NavListNode.h"
#include "meta_ham/PlaylistSort.h"
#include "utl/Symbol.h"

class PlaylistTypeCmp : public NavListItemSortCmp {
public:
    virtual ~PlaylistTypeCmp();
    virtual int Compare(NavListItemSortCmp const *, NavListNodeType) const;

    PlaylistTypeCmp(int type, const char *name) : mType(type), mName(name) {}

    int mType;
    const char *mName;
};

class PlaylistSortByType : public PlaylistSort {
public:
    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;

    PlaylistSortByType() {
        static Symbol by_type("by_type");
        mSortName = by_type;
    }
};
