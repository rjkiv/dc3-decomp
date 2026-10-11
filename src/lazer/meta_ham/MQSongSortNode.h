#pragma once
#include "NavListNode.h"
#include "SongRecord.h"
#include "utl/Symbol.h"
class MQSongHeaderNode : public NavListHeaderNode {
public:
    MQSongHeaderNode(NavListItemSortCmp *, Symbol, bool);

    DataNode Handle(DataArray *, bool);
    virtual ~MQSongHeaderNode() {}
    virtual Symbol OnSelect();
    virtual Symbol OnSelectDone();
    virtual void OnHighlight();
    virtual void OnUnHighlight();
    virtual NavListSortNode *GetFirstActive();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual bool IsActive() const;
    virtual const char *GetAlbumArtPath();
    virtual void SetCollapseStateIcon(bool) const;
    virtual void Renumber(std::vector<NavListSortNode *> &);

protected:
    int mCount; // 0x58
    bool mIsHighlighted; // 0x5c
};

class MQSongSortNode : public NavListItemNode {
public:
    MQSongSortNode(NavListItemSortCmp *cmp, Symbol s1, Symbol s2)
        : NavListItemNode(cmp), mSong(s1), mCharacter(s2) {};
    virtual ~MQSongSortNode();
    virtual Symbol OnSelect();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual void Custom(UIListCustom *, Hmx::Object *) const;

    Symbol mSong; // 0x48
    Symbol mCharacter; // 0x4c
};
