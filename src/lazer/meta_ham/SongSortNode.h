#pragma once

#include "meta_ham/NavListNode.h"
#include "meta_ham/SongRecord.h"
#include "utl/Symbol.h"

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
    virtual char const *GetAlbumArtPath();
    virtual void Renumber(std::vector<NavListSortNode *> &);
    virtual void UpdateItemCount(NavListItemNode *);
    virtual void SetItemCountString(UILabel *) const;
    virtual void SetCollapseStateIcon(bool) const;

protected:
    int mDiscCount; // 0x58
    int mDownloadCount; // 0x5c
};

class SongSortNode : public NavListItemNode {
public:
    SongSortNode(NavListItemSortCmp *cmp, SongRecord *song)
        : NavListItemNode(cmp), mSongRecord(song), mInPlaylist(0) {}

    virtual DataNode Handle(DataArray *, bool);
    virtual Symbol GetToken() const;
    virtual Symbol OnSelect();
    virtual Symbol Select();
    virtual void OnContentMounted(const char *, const char *);
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual void Custom(UIListCustom *, Hmx::Object *) const;

    void SetInPlaylist(bool);
    bool IsCoverSong(Symbol) const;
    bool IsMedley() const;
    bool IsFake() const;
    const char *GetArtist() const;

    const SongRecord *GetSongRecord() const { return mSongRecord; }

private:
    SongRecord *mSongRecord; // 0x48
    bool mInPlaylist; // 0x4c
};

class SongFunctionNode : public NavListFunctionNode {
public:
    SongFunctionNode(NavListItemSortCmp *cmp, Symbol sym, const char *c)
        : NavListFunctionNode(cmp, sym, c) {}
    virtual ~SongFunctionNode() {}
    virtual Symbol OnSelect();
    virtual void OnHighlight();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual bool IsActive() const;
};
