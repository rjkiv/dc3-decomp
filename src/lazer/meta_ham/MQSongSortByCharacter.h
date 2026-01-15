#pragma once
#include "MQSongSort.h"
#include "NavListNode.h"
class MQSongCharCmp : public NavListItemSortCmp {
public:
    MQSongCharCmp(const char *c, const char *c2) : unk4(c), unk8(c2){}
    virtual int Compare(const NavListItemSortCmp *, NavListNodeType) const;

    const char *unk4;
    const char *unk8;
};

class MQSongSortByCharacter : public MQSongSort {
public:
    MQSongSortByCharacter();
    virtual ~MQSongSortByCharacter();

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
};
