#pragma once
#include "NavListNode.h"
#include "NavListSort.h"

struct SortNodeFind {
    SortNodeFind(const NavListSortNode *);
    bool operator()(const NavListSortNode *) const;

    Symbol mToken; // 0x0
    NavListNodeType mType; // 0x4
};

struct NodeFind {
    NodeFind(Symbol s) : unk0(s) {}
    bool operator()(const NavListNode *n) { return n->GetToken() == unk0; }

    Symbol unk0;
};

class ChallengeSort : public NavListSort {
public:
    ChallengeSort();
    virtual ~ChallengeSort() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual void BuildTree();
    virtual void DeleteItemList();
    virtual void BuildItemList();
    virtual void SetHighlightedIx(int);
    virtual void SetHighlightItem(const NavListSortNode *);
    virtual void UpdateHighlight();
    virtual void OnSelectShortcut(int);

protected:
};
