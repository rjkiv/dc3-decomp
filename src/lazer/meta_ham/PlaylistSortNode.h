#pragma once
#include "NavListNode.h"
#include "Playlist.h"
#include "meta_ham/Playlist.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIListCustom.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

class PlaylistSortNode : public NavListItemNode {
public:
    virtual Symbol Select();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual void Custom(UIListCustom *, Hmx::Object *) const;
    virtual Symbol OnSelect();
    virtual void OnContentMounted(char const *, char const *);

    Playlist *GetPlaylist() { return unk48; }

    PlaylistSortNode(NavListItemSortCmp *cmp, Playlist *p)
        : NavListItemNode(cmp), unk48(p) {}

protected:
    Playlist *unk48;
};

class PlaylistHeaderNode : public NavListHeaderNode {
public:
    // Hmx::Object
    virtual ~PlaylistHeaderNode() {}
    virtual DataNode Handle(DataArray *, bool);

    // NavListSortNode
    virtual Symbol OnSelect();
    virtual Symbol Select();
    virtual Symbol OnSelectDone();
    virtual void OnHighlight();
    NavListSortNode *GetFirstActive();
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual bool IsActive() const;
    char const *GetAlbumArtPath();
    virtual void Renumber(std::vector<NavListSortNode *> &);
    virtual void UpdateItemCount(NavListItemNode *);
    virtual void SetItemCountString(UILabel *) const;

    PlaylistHeaderNode(NavListItemSortCmp *, Symbol, bool);

protected:
    int mChallengeCount; // 0x58
};
