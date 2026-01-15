#pragma once
#include "NavListNode.h"
#include "NavListSort.h"

class MQSongSort : public NavListSort {
public:
    MQSongSort();
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual void BuildTree();
    virtual void DeleteItemList(); // 0x74
    virtual void BuildItemList();
    virtual void SetHighlightedIx(int);
    virtual void SetHighlightItem(const NavListSortNode *);
    virtual void UpdateHighlight();
    virtual void OnSelectShortcut(int);
};
