#pragma once
#include "MQSongSort.h"
#include "NavListNode.h"
class MQSongCharCmp : public NavListItemSortCmp {
public:
    MQSongCharCmp(Symbol song, Symbol character) : mSong(song), mCharacter(character) {}
    virtual int Compare(const NavListItemSortCmp *, NavListNodeType) const;

    Symbol mSong; // 0x4
    Symbol mCharacter; // 0x8
};

class MQSongSortByCharacter : public MQSongSort {
public:
    MQSongSortByCharacter() {
        static Symbol by_character("by_character");
        mSortName = by_character;
    }
    virtual ~MQSongSortByCharacter();

    virtual NavListItemNode *NewItemNode(void *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *, NavListItemNode *) const;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const;
};
