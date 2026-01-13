#pragma once

#include "meta_ham/NavListNode.h"
#include "meta_ham/SongRecord.h"

class SongHeaderNode : public NavListHeaderNode {
public:
    SongHeaderNode(NavListItemSortCmp *, Symbol, bool);
    virtual ~SongHeaderNode();
    virtual DataNode Handle(DataArray *, bool);
    virtual Symbol OnSelect();
    virtual Symbol Select();
    virtual Symbol OnSelectDone();
    virtual void OnHighlight();
    virtual int GetItemCount();
    virtual NavListSortNode *GetFirstActive();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual bool IsActive() const;
    virtual void Renumber(std::vector<NavListSortNode *> &);
    virtual void UpdateItemCount(NavListItemNode *);
    virtual void SetItemCountString(UILabel *) const;
    virtual void SetCollapseStateIcon(bool) const;

private:
    u32 mDiscSongs;
    u32 mDLCSongs;
};

class SongSortNode : public NavListItemNode {
public:
    SongSortNode(NavListItemSortCmp *cmp, SongRecord *song)
        : NavListItemNode(cmp), unk_0x48(song), unk_0x4C(0) {}

    virtual DataNode Handle(DataArray *, bool);
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual void Custom(UIListCustom *, Hmx::Object *) const;
    virtual Symbol Select();

    void SetInPlaylist(bool);
    bool IsCoverSong(Symbol) const;
    bool IsMedley() const;
    bool IsFake() const;
    const char *GetArtist() const;

    const SongRecord *Record() const { return unk_0x48; }

private:
    SongRecord *unk_0x48;
    bool unk_0x4C;
};

class SongFunctionNode : public NavListFunctionNode {
public:
    virtual Symbol OnSelect();
    virtual void OnHighlight();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual bool IsActive() const;
};