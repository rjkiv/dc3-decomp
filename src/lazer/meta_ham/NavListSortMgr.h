#pragma once

#include "meta/SongPreview.h"
#include "NavListNode.h"
#include "NavListSort.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/System.h"
#include "ui/UIComponent.h"
#include "ui/UIListCustom.h"
#include "ui/UIListProvider.h"
class NavListSortMgr : public UIListProvider, Hmx::Object, ContentMgr::Callback {
public:
    NavListSortMgr(SongPreview &);
    virtual ~NavListSortMgr();
    // Hmx::Object

    // UIListProvider
    virtual DataNode Handle(DataArray *, bool);
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual void Custom(int, int, UIListCustom *, Hmx::Object *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int DataIndex(Symbol s) const;
    virtual int NumData() const;
    virtual bool IsActive(int) const;
    virtual UIComponent::State ComponentStateOverride(int, int, UIComponent::State) const;
    virtual bool IsHeader(int);
    virtual void UnHighlightCurrent();
    virtual void ClearIconLabels();
    // ContentMgr::Callback
    virtual void ContentMounted(const char *, const char *);


    void StopPreview();
    void SetHeaderMode(bool);
    void SetEnteringHeaderMode(bool);
    void SetExitingHeaderMode(bool);
    NavListSortNode *GetHighlightItem() { return mSorts[mCurrentSortIdx]->GetUnk50(); };
    void OnHighlightChanged();
    void OnExit();
    void OnUnload();
    NavListSort *GetCurrentSort();
    Symbol GetCurrentSortName();
    Symbol GetHeaderSymbolFromChildSymbol(Symbol);
    void DoUncollapse();
    Symbol OnSelect(int);
    Symbol OnSelectDone(int);
    void StartPreview(int, TexMovie *);
    Symbol OnGetToken(int);
    bool IsIndexHeader(int);
    virtual int GetListIndexFromHeaderIndex(int);
    virtual Symbol GetFirstChildSymbolFromHeaderSymbol(Symbol);
    int GetHeaderIndexFromListIndex(int);
    int GetHeaderIndexFromChildListIndex(int);
    bool IsHeaderCollapsed(Symbol);
    void SetSort(int);
    void SetSort(Symbol);
    void SetHeaderCollapsed(Symbol);
    void SetHeaderUncollapsed(Symbol);
    void AddHeaderIndex(int);
    void FinalizeHeaders();
    void ClearHeaders();

    bool IsInHeaderMode() { return mHeaderMode; }

protected:
    std::vector<NavListSort *> mSorts; // 0x34
    int mCurrentSortIdx; // 0x40
    DataArray *unk44; // 0x44
    bool unk48; // 0x48 mHighlightSaved maybe?
    SongPreview *mSongPreview; // 0x4c
    bool mHeaderMode; // 0x50
    bool mEnteringHeaderMode; // 0x51
    bool mExitingHeaderMode; // 0x52
    std::vector<int> mHeadersA; // 0x54
    std::vector<int> mHeadersB; // 0x60
    bool mHeadersSelectable; // 0x6c
    std::list<Symbol> unk70; // 0x70 mSortNames?
};
