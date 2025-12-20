#pragma once
#include "NavListSort.h"

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
