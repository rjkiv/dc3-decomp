#pragma once
#include "meta_ham/NavListNode.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

class FitnessCalorieSortNode : public NavListSortNode {
public:
    virtual Symbol GetToken() const;
    virtual Symbol OnSelect();
    virtual void Text(UIListLabel *, UILabel *) const;

    FitnessCalorieSortNode(NavListItemSortCmp *, int);

protected:
    Symbol unk44;
    int unk48;
};

class FitnessCalorieHeaderNode : public NavListHeaderNode {
public:
    virtual Symbol OnSelect();
    virtual Symbol OnSelectDone();
    virtual void OnHighlight();
    virtual NavListSortNode *GetFirstActive();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual bool IsActive() const;
    virtual void Renumber(std::vector<NavListSortNode *> &);
    virtual void SetCollapseStateIcon(bool) const;

    FitnessCalorieHeaderNode(NavListItemSortCmp *, Symbol, bool);

protected:
    int unk58;
};
