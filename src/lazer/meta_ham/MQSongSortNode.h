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

    int unk58; // 0x58
    bool unk5c; // 0x5c
};

class MQSongSortNode : public NavListItemNode {
public:
    MQSongSortNode(NavListItemSortCmp *cmp, Symbol s1, Symbol s2)
        : NavListItemNode(cmp), unk48(s1), unk4c(s2) {};
    virtual ~MQSongSortNode();
    virtual Symbol OnSelect();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual void Custom(UIListCustom *, Hmx::Object *) const;

    void SetUnk4C(Symbol s) { unk4c = s; }
    Symbol GetUnk4C() const { return unk4c; }

protected:
    Symbol unk48; // 0x48
    Symbol unk4c; // 0x4c
};
