#pragma once
#include "NavListNode.h"
class MQSongHeaderNode : public NavListHeaderNode {
public:
    MQSongHeaderNode(NavListItemSortCmp *, Symbol, bool);

    DataNode Handle(DataArray *, bool);
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

