#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

enum NavListNodeType {
    // stole these from RB3 lmao
    kNodeNone = 0,
    kNodeShortcut = 1,
    kNodeHeader = 2,
    kNodeSubheader = 3,
    kNodeItem = 4,
    kNodeFunction = 5,
    kNodeSetlist = 6,
    kNodeStoreSong = 7
};

class NavListItemSortCmp {
public:
    NavListItemSortCmp() {}
    virtual ~NavListItemSortCmp() {}
    virtual int Compare(const NavListItemSortCmp *, NavListNodeType) const = 0;
    virtual bool HasSubheader() const { return false; }
    virtual const class DifficultyCmp *GetDifficultyCmp() const;
    virtual const class SongCmp *GetSongCmp() const;
    virtual const class ArtistCmp *GetArtistCmp() const; // tentative
    virtual const class DecadeCmp *GetDecadeCmp() const; // tentative
    virtual const class VenueCmp *GetVenueCmp() const; // tentative
    virtual const class DateCmp *GetDateCmp() const; // tentative
    virtual const class LocationCmp *GetLocationCmp() const; // tentative - 0x24
    virtual const class AlbumCmp *GetAlbumCmp() const; // tentative
    virtual const class VocalPartsCmp *GetVocalPartsCmp() const; // tentative
    virtual const class PlaylistTypeCmp *GetPlaylistTypeCmp() const;
    virtual const class ChallengeScoreCmp *GetChallengeScoreCmp() const;
    virtual const class MQSongCharCmp *GetMQSongCharCmp() const; // tentative
    virtual const class FitnessCalorieSortCmp *GetFitnessCalorieSortCmp() const;
};

class NavListNode : public Hmx::Object {
public:
    NavListNode(NavListItemSortCmp *);
    virtual ~NavListNode();
    virtual NavListNodeType GetType() const = 0; // 0x58
    virtual Symbol GetToken() const = 0; // 0x5c
    virtual bool LocalizeToken() const { return true; }
    virtual const DateTime *GetDateTime() const { return nullptr; }

    int Compare(const NavListNode *, NavListNodeType) const;
    void GetID(DataArray *);
    NavListItemSortCmp *GetCmp() { return mCmp; }
    NavListNode *Parent() const { return mParent; }
protected:
    NavListItemSortCmp *mCmp; // 0x2c
    NavListNode *mParent; // 0x30
};

class NavListSort;
class NavListShortcutNode;

class NavListSortNode : public NavListNode {
public:
    NavListSortNode(NavListItemSortCmp *);
    virtual ~NavListSortNode();
    virtual DataNode Handle(DataArray *, bool);
    virtual void FinishSort(NavListSort *);
    virtual Symbol OnSelect() = 0;
    virtual Symbol Select() = 0;
    virtual Symbol OnSelectDone() { return gNullStr; }
    virtual void OnHighlight() = 0;
    virtual void OnUnHighlight() = 0;
    virtual void SetCollapseIconLabel(UILabel *) = 0;
    virtual int GetItemCount() = 0;
    virtual NavListSortNode *GetFirstActive() = 0; // 0x88
    virtual void OnContentMounted(const char *, const char *) {}
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual void Custom(UIListCustom *, Hmx::Object *) const;
    virtual RndMat *Mat(UIListMesh *) const { return nullptr; }
    virtual bool IsEnabled() const = 0; // 0x9c
    virtual bool IsActive() const = 0; // 0xa0
    virtual const char *GetAlbumArtPath() = 0; // 0xa4
    virtual void DeleteAll();
    virtual void Renumber(std::vector<NavListSortNode *> &);
    virtual void FinishBuildList(NavListSort *);

    void SetShortcut(NavListShortcutNode *);
    int StartIndex() const { return mStartIx; }
    void SetStartIndex(int idx) { mStartIx = idx; }
    NavListShortcutNode *GetShortcut() const { return mShortcut; }
    const std::list<NavListSortNode *> &Children() const { return mChildren; }
    int GetStartIx() { return mStartIx; }

protected:
    std::list<NavListSortNode *> mChildren; // 0x34
    NavListShortcutNode *mShortcut; // 0x3c
    int mStartIx; // 0x40
};

class NavListShortcutNode : public NavListNode {
public:
    NavListShortcutNode(NavListItemSortCmp *, Symbol, bool);
    virtual ~NavListShortcutNode();
    virtual NavListNodeType GetType() const { return kNodeShortcut; }
    virtual Symbol GetToken() const { return mToken; }
    virtual bool LocalizeToken() const { return mLocalizeToken; }
    virtual const DateTime *GetDateTime() const { return mDateTime; }
    virtual void DeleteAll();

    void Insert(class NavListItemNode *, class NavListSort *);
    void
    InsertHeaderRange(class NavListItemNode **, class NavListItemNode **, class NavListSort *);
    NavListSortNode *GetFirstActive();
    bool IsActive() const;
    void FinishBuildList(NavListSort *);
    void FinishSort(NavListSort *);
    void Renumber(std::vector<NavListSortNode *> &);
    const std::list<NavListSortNode *> &Children() const { return mChildren; }

protected:
    Symbol mToken; // 0x34
    bool mLocalizeToken; // 0x38
    DateTime *mDateTime; // 0x3c
    std::list<NavListSortNode *> mChildren; // 0x40
};

class NavListItemNode : public NavListSortNode {
public:
    NavListItemNode(NavListItemSortCmp *cmp) : NavListSortNode(cmp) {}
    virtual ~NavListItemNode() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual NavListNodeType GetType() const { return kNodeItem; }
    virtual Symbol GetToken() const { return gNullStr; }

    virtual void FinishSort(NavListSort *) {}
    virtual Symbol OnSelect() { return gNullStr; }
    virtual Symbol Select() { return gNullStr; }
    virtual Symbol OnSelectDone() { return gNullStr; }
    virtual void OnHighlight() {}
    virtual void OnUnHighlight() {}
    virtual void SetCollapseIconLabel(UILabel *) {}
    virtual int GetItemCount() { return 1; }
    virtual NavListSortNode *GetFirstActive();
    virtual void OnContentMounted(const char *, const char *) {}
    virtual void Text(UIListLabel *, UILabel *) const;
    virtual void Custom(UIListCustom *, Hmx::Object *) const {}
    virtual RndMat *Mat(UIListMesh *) const;
    virtual bool IsEnabled() const { return IsEnabled(); } // lmao what
    virtual bool IsActive() const { return true; }
    virtual const char *GetAlbumArtPath() { return nullptr; }
    virtual void Renumber(std::vector<NavListSortNode *> &);
    virtual void FinishBuildList(NavListSort *);

    bool IsHeader() const;
    Symbol HeaderText() const;
    bool UseQuickplayPerformer();

protected:
    Symbol mHeader; // 0x44
};

class NavListFunctionNode : public NavListSortNode {
public:
    NavListFunctionNode(NavListItemSortCmp *, Symbol, const char *);
    virtual ~NavListFunctionNode() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual NavListNodeType GetType() const { return kNodeFunction; }
    virtual Symbol GetToken() const { return unk4c; }
    virtual Symbol Select();
    virtual bool IsEnabled() const { return IsEnabled(); } // lmao what
    virtual bool IsActive() const { return false; }
    virtual const char *GetAlbumArtPath() { return unk44.c_str(); }
    virtual void Renumber(std::vector<NavListSortNode *> &);
    virtual int GetTier() const { return -1; }
    virtual const char *GetShortcutStr() { return gNullStr; }
    virtual const char *GetArtist() const { return gNullStr; }

protected:
    String unk44;
    Symbol unk4c;
};

class NavListHeaderNode : public NavListSortNode {
public:
    NavListHeaderNode(NavListItemSortCmp *, Symbol, bool);
    virtual ~NavListHeaderNode();
    virtual DataNode Handle(DataArray *, bool);
    virtual NavListNodeType GetType() const { return kNodeHeader; }
    virtual Symbol GetToken() const { return mToken; }
    virtual bool LocalizeToken() const { return mLocalizeToken; }
    virtual const DateTime *GetDateTime() const { return mDateTime; }
    virtual void FinishSort(NavListSort *sort) { NavListSortNode::FinishSort(sort); }
    virtual Symbol OnSelect() { return gNullStr; }
    virtual Symbol Select();
    virtual Symbol OnSelectDone() { return gNullStr; }
    virtual void OnHighlight() {}
    virtual void OnUnHighlight() {}
    virtual void SetCollapseIconLabel(UILabel *label) { mCollapseIconLabel = label; }
    virtual int GetItemCount() { return 0; }
    virtual NavListSortNode *GetFirstActive() { return nullptr; }
    virtual bool IsEnabled() const;
    virtual bool IsActive() const { return IsActive(); } // ok then
    virtual const char *GetAlbumArtPath() { return nullptr; }
    virtual void Insert(NavListItemNode *, NavListSort *);
    virtual void UpdateItemCount(NavListItemNode *) {}
    virtual UILabel *GetCollapseIconLabel() const { return mCollapseIconLabel; }
    virtual Symbol SelectChildren(std::list<NavListSortNode *> &, int);
    virtual void SetItemCountString(UILabel *) const;
    virtual void SetCollapseStateIcon(bool) const;

protected:
    bool unk44;
    Symbol mToken; // 0x48
    bool mLocalizeToken; // 0x4c
    DateTime *mDateTime; // 0x50
    UILabel *mCollapseIconLabel; // 0x54
};
