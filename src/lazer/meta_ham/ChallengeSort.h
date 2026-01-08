#pragma once
#include "NavListNode.h"
#include "NavListSort.h"

struct SortNodeFind {
public:
    SortNodeFind(const NavListSortNode *);
    bool operator()(const NavListSortNode *) const;

protected:
    Symbol mToken; // 0x0
    NavListNodeType mType; // 0x4
};

class ChallengeSort : public NavListSort {
public:
    ChallengeSort();
    ~ChallengeSort();

    void Text(int, int, UIListLabel *, UILabel *) const;
    void BuildTree();
    void DeleteItemList();
    void BuildItemList();
    void SetHighlightedIx(int);
    void SetHighlightItem(const NavListSortNode *);
    void UpdateHighlight();
    void OnSelectShortcut(int);

protected:
};
