#pragma once
#include "NavListSort.h"
#include "meta_ham/NavListNode.h"

class PlaylistSort : public NavListSort {
public:
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;

    // NavListSort
    virtual void BuildTree();
    virtual void DeleteItemList();
    virtual void BuildItemList();
    virtual void SetHighlightedIx(int);
    virtual void SetHighlightItem(NavListSortNode const *);
    virtual void UpdateHighlight();
    virtual void OnSelectShortcut(int);

    PlaylistSort();
};

class PlaylistSortByType : public PlaylistSort {
public:
    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;

    PlaylistSortByType();

    static Symbol unk58;
};
