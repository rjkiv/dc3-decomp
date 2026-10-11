#pragma once
#include "meta_ham/NavListNode.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

// class not in pdb :[

class FitnessCalorieSortNode : public NavListItemNode {
public:
    virtual Symbol GetToken() const;
    virtual Symbol OnSelect();
    virtual void Text(UIListLabel *, UILabel *) const;

    FitnessCalorieSortNode(NavListItemSortCmp *cmp, int i)
        : NavListItemNode(cmp), mCalorieNodeIndex(i) {}
    int GetCalorieNodeIndex() const { return mCalorieNodeIndex; }

protected:
    int mCalorieNodeIndex; // 0x48
};

class FitnessCalorieHeaderNode : public NavListHeaderNode {
public:
    virtual ~FitnessCalorieHeaderNode() {}
    virtual Symbol OnSelect();
    virtual Symbol OnSelectDone();
    virtual void OnHighlight();
    virtual void OnUnHighlight();
    virtual NavListSortNode *GetFirstActive();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual bool IsActive() const;
    virtual void Renumber(std::vector<NavListSortNode *> &);
    virtual void SetCollapseStateIcon(bool) const;

    FitnessCalorieHeaderNode(NavListItemSortCmp *, Symbol, bool);

protected:
    int mIndex; // 0x58
};
