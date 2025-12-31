
#pragma once
#include "meta_ham/NavListNode.h"
#include "meta_ham/NavListSort.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"

class SongSort : public NavListSort {
public:
    SongSort();
    virtual ~SongSort() {};
    virtual void BuildTree();
    virtual void DeleteItemList();
    virtual void BuildItemList();
    virtual void SetHighlightedIx(int);
    virtual void SetHighlightItem(const NavListSortNode *);
    virtual void UpdateHighlight();
    virtual void OnSelectShortcut(int);
    virtual void Text(int, int, UIListLabel *, UILabel *) const;

    Symbol DetermineHeaderSymbolFromSong(Symbol);

};
