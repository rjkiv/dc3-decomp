#pragma once
#include "meta_ham/FitnessCalorieSort.h"
#include "meta_ham/NavListNode.h"

class FitnessCalorieSortCmp : public NavListItemSortCmp {
public:
    virtual ~FitnessCalorieSortCmp();
    virtual int Compare(NavListItemSortCmp const *, NavListNodeType) const;

    FitnessCalorieSortCmp() {}
};

class FitnessCalorieSortByCalorie : public FitnessCalorieSort {
public:
    virtual ~FitnessCalorieSortByCalorie();
    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;

    FitnessCalorieSortByCalorie();
};
