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
#include "utl/Symbol.h"
class NavListSortMgr : public UIListProvider,
                       public Hmx::Object,
                       public ContentMgr::Callback {
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
    virtual bool HeadersSelectable(); // 0x6c
    virtual bool SelectionIs(Symbol); // 0x70
    virtual bool DataIs(int, Symbol); // 0x74
    virtual Symbol MoveOn(); // 0x78
    virtual void OnEnter(); // 0x7c
    virtual int GetListIndexFromHeaderIndex(int); // 0x80
    virtual Symbol GetFirstChildSymbolFromHeaderSymbol(Symbol); // 0x84
    virtual DataNode OnCancel(); // 0x88
    // ContentMgr::Callback
    virtual void ContentMounted(const char *, const char *); // 0x8c

    void StopPreview();
    void SetHeaderMode(bool);
    void SetEnteringHeaderMode(bool);
    void SetExitingHeaderMode(bool);
    NavListSortNode *GetHighlightItem();
    void OnHighlightChanged();
    void OnExit();
    void OnUnload();
    NavListSort *GetCurrentSort();
    NavListSort *GetCurrentSortHandle();
    Symbol GetCurrentSortName();
    Symbol GetHeaderSymbolFromChildSymbol(Symbol);
    void DoUncollapse();
    Symbol OnSelect(int);
    Symbol OnSelectDone(int);
    void StartPreview(int, TexMovie *);
    Symbol OnGetToken(int);
    bool IsIndexHeader(int);
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
    int FirstDataIndex(Symbol);
    bool IsDisabled(int);
    void SetHighlightedIx(int);
    int GetHeaderCount();
    void SortWithHeaders(int);
    void SetHeaderMode(int);
    void NextSort();

    bool &IsInHeaderMode() { return mHeaderMode; }
    bool &EnteringHeaderMode() { return mEnteringHeaderMode; }
    bool &GetHeadersSelectable() { return mHeadersSelectable; }
    bool &ExitingHeaderMode() { return mExitingHeaderMode; }
    std::vector<NavListSort *> &Sorts() { return mSorts; };
    std::vector<int> &GetHeadersA() { return mHeadersA; };
    std::vector<int> &GetHeadersB() { return mHeadersB; };
    int &GetHeadersBAtIdx(int idx) { return mHeadersB[idx]; };
    int GetCurrentSortIdx() const { return mCurrentSortIdx; };
    SongPreview *GetSongPreview() { return mSongPreview; };

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
