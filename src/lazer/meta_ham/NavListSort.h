#pragma once
#include "NavListNode.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListProvider.h"
#include "meta_ham/NavListNode.h"

class NavListSort : public UIListProvider, public Hmx::Object {
public:
    NavListSort();
    virtual ~NavListSort() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual int NumData() const { return unk30.size(); }
    virtual bool IsActive(int idx) const { return unk30[idx]->IsActive(); }
    virtual void BuildTree() = 0;
    virtual void DeleteItemList();
    virtual void BuildItemList() = 0;
    virtual void SetHighlightedIx(int) = 0;
    virtual void SetHighlightItem(const NavListSortNode *) = 0;
    virtual void UpdateHighlight();
    virtual void OnSelectShortcut(int);
    virtual bool GetHeaderSelectable() { return false; }
    virtual void Init() {}
    virtual NavListItemNode *NewItemNode(void *) const = 0;
    virtual NavListShortcutNode *NewShortcutNode(NavListItemNode *) const = 0;
    virtual NavListHeaderNode *
    NewHeaderNode(NavListItemNode *, NavListItemNode *) const = 0;
    virtual NavListHeaderNode *NewHeaderNode(NavListItemNode *) const = 0;

    int GetCurrentShortcut();
    void ChangeHighlightHeader(int);
    const char *HighlightTokenStr() const;
    NavListSortNode *GetNode(Symbol) const;
    int GetDataCount() const;
    void DeleteTree();
    bool SetHighlightID(DataArray *);

    NavListSortNode *GetUnk50() { return unk50; }
    NavListSortNode *GetUnk54() { return unk54; }
    void SetUnk50(NavListSortNode *sortnode) { unk50 = sortnode; }
    void SetUnk54(NavListSortNode *sortnode) { unk54 = sortnode; }
    Symbol GetSortName() { return mSortName; }
    void SetSortName(Symbol name) { mSortName = name; }
    NavListSortNode *GetListFromIdx(int idx) { return mList[idx]; }
    std::vector<NavListSortNode*> &GetList() { return mList; }
    int GetListSize() { return mList.size(); }

protected:
    std::vector<NavListShortcutNode *> unk30;
    std::list<NavListSortNode *> unk3c;
    std::vector<NavListSortNode *> mList; // 0x44
    NavListSortNode *unk50; // 0x50
    NavListSortNode *unk54; // 0x54
    Symbol mSortName; // 0x58
};
