#pragma once
#include "MQSongSort.h"
#include "NavListNode.h"
class MQSongCharCmp : public NavListItemSortCmp {
public:
    MQSongCharCmp();
    virtual ~MQSongCharCmp();

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
