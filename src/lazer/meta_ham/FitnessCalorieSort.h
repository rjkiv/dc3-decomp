#pragma once
#include "NavListSort.h"
#include "meta_ham/NavListNode.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"

class FitnessCalorieSort : public NavListSort {
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

    FitnessCalorieSort();
};
