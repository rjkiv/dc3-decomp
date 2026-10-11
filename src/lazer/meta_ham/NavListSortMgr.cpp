#include "NavListSortMgr.h"

#include "NavListSort.h"
#include "macros.h"
#include "meta/SongPreview.h"
#include "meta_ham/NavListNode.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/System.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

NavListSortMgr::NavListSortMgr(SongPreview &songprev)
    : mSongPreview(songprev), mSortIndex(0), mSavedHighlight(false), mHeaderMode(false),
      mEnteringHeaderMode(false), mExitingHeaderMode(false), mSortWithHeaders(false) {
    mSavedHighlightID = new DataArray(0);
};

NavListSortMgr::~NavListSortMgr() {
    DeleteAll(mSorts);
    mSavedHighlightID->Release();
}

void NavListSortMgr::StopPreview() { mSongPreview.Start(gNullStr, 0); }

void NavListSortMgr::SetHeaderMode(bool b) { mHeaderMode = b; }

void NavListSortMgr::SetEnteringHeaderMode(bool b) { mEnteringHeaderMode = b; }

void NavListSortMgr::SetExitingHeaderMode(bool b) { mExitingHeaderMode = b; }

void NavListSortMgr::AddHeaderIndex(int idx) {
    if (mSortWithHeaders) {
        mHeaderIndicesAmongAll.push_back(idx);
        mHeaderIndicesAlone.push_back(idx);
    }
}

int NavListSortMgr::GetHeaderIndexFromChildListIndex(int i1) {
    int ret = -1;
    int numBs = mHeaderIndicesAlone.size();
    for (int i = 0; i < numBs; i++) {
        int cur = mHeaderIndicesAlone[i];
        if (cur == i1) {
            ret = i1;
            break;
        } else if (cur > i1) {
            if (i == 0) {
                ret = mHeaderIndicesAlone[0];
            } else {
                ret = mHeaderIndicesAlone[i - 1];
            }
            break;
        } else if (cur < i1 && i == numBs - 1) {
            ret = mHeaderIndicesAlone[numBs - 1];
            break;
        }
    }
    return ret;
}

bool NavListSortMgr::IsHeader(int idx) {
    if (0 <= idx && mHeaderIndicesAmongAll.size() > idx) {
        return mHeaderIndicesAmongAll[idx] != false;
    }
    return false;
}

void NavListSortMgr::ClearHeaders() {
    mHeaderIndicesAmongAll.clear();
    mHeaderIndicesAlone.clear();
}

Symbol NavListSortMgr::GetHeaderSymbolFromChildSymbol(Symbol sym) {
    NavListSort *sort = mSorts[mSortIndex];
    NavListSortNode *sortNode = sort->GetNode(sym);
    if (!sortNode) {
        return gNullStr;
    } else {
        NavListHeaderNode *node =
            dynamic_cast<NavListHeaderNode *>(sort->GetNode(sym)->GetParent());
        if (!node) {
            return sortNode->GetToken();
        } else {
            return node->GetToken();
        }
    }
}

NavListSort *NavListSortMgr::GetCurrentSort() { return mSorts[mSortIndex]; }

Symbol NavListSortMgr::GetCurrentSortName() {
    NavListSort *pCurrentSort = mSorts[mSortIndex];
    if (!pCurrentSort) {
        MILO_ASSERT(pCurrentSort, 0x19c);
    }
    return pCurrentSort->GetSortName();
}

void NavListSortMgr::SetSort(int idx) {
    if (idx >= 0 && idx < mSorts.size()) {
        mUncollapsedHeaderIDs.clear();
        ClearIconLabels();
        mSortIndex = idx;
        mSorts[idx]->BuildItemList();
        mSorts[mSortIndex]->SetHighlightedIx(0);
        mSorts[mSortIndex]->UpdateHighlight();
    }
}

void NavListSortMgr::SetSort(Symbol sym) {
    for (int i = 0; i < mSorts.size(); i++) {
        if (sym == mSorts[i]->GetSortName()) {
            SetSort(i);
            return;
        }
    }
    MILO_NOTIFY("Failed to find a sort for the symbol %s", sym);
}

void NavListSortMgr::SetHeaderUncollapsed(Symbol sym) {
    FOREACH (it, mUncollapsedHeaderIDs) {
        if (*it == sym) {
            return;
        }
    }
    mUncollapsedHeaderIDs.push_back(sym);
}

void NavListSortMgr::SetHeaderCollapsed(Symbol sym) {
    FOREACH (it, mUncollapsedHeaderIDs) {
        if (*it == sym) {
            mUncollapsedHeaderIDs.erase(it);
            return;
        }
    }
}

bool NavListSortMgr::IsHeaderCollapsed(Symbol sym) {
    FOREACH (it, mUncollapsedHeaderIDs) {
        if (*it == sym) {
            return false;
        }
    }
    return true;
}

bool NavListSortMgr::IsIndexHeader(int idx) {
    if (idx >= 0 && mHeaderIndicesAmongAll.size() >= idx) {
        return mHeaderIndicesAmongAll[idx] != 0;
    }
    return false;
}

void NavListSortMgr::Text(int i1, int i2, UIListLabel *listlabel, UILabel *label) const {
    if (0 <= i2 && i2 < NumData()) {
        if (listlabel->Matches("header_collapse")) {
            mSorts[mSortIndex]->GetListFromIdx(i2)->SetCollapseIconLabel(label);
        }
        mSorts[mSortIndex]->GetListFromIdx(i2)->Text(listlabel, label);
    }
}

void NavListSortMgr::UnHighlightCurrent() {
    if (mSorts[mSortIndex]->GetUnk54()) {
        mSorts[mSortIndex]->GetUnk54()->OnUnHighlight();
        mSorts[mSortIndex]->SetUnk54(0);
    }
}

void NavListSortMgr::ContentMounted(const char *c1, const char *c2) {
    NavListSortNode *node = mSorts[mSortIndex]->GetHighlightItem();
    if (!node) {
        return;
    }
    node->OnContentMounted(c1, c2);
}

void NavListSortMgr::DoUncollapse() {
    if (!IsInHeaderMode()) {
        MILO_ASSERT(IsInHeaderMode(), 0x264);
    }
    mHeaderMode = false;
    OnEnter();
    mSorts[mSortIndex]->BuildItemList();
}

UIComponent::State
NavListSortMgr::ComponentStateOverride(int i1, int i2, UIComponent::State state) const {
    if (!mSorts[mSortIndex]->GetListFromIdx(i2)->IsEnabled()) {
        return UIComponent::State::kDisabled;
    }
    return state;
}

int NavListSortMgr::GetListIndexFromHeaderIndex(int idx) {
    int size = mHeaderIndicesAlone.size();
    if (idx < 0) {
        if (0 < size) {
            return mHeaderIndicesAlone.front();
        }
        return 0;
    }
    if (idx >= size) {
        if (0 < size) {
            return mHeaderIndicesAlone[size - 1];
        }
        return 0;
    }
    return mHeaderIndicesAlone[idx];
}

void NavListSortMgr::OnExit() {
    NavListSortNode *node = GetHighlightItem();
    if (node) {
        node->GetID(mSavedHighlightID);
        mSavedHighlight = true;
    }
}

void NavListSortMgr::Custom(int i1, int i2, UIListCustom *uilist, Hmx::Object *obj) const {
    if (0 <= i2 && i2 < NumData()) {
        mSorts[mSortIndex]->GetListFromIdx(i2)->Custom(uilist, obj);
    }
}

RndMat *NavListSortMgr::Mat(int i1, int i2, UIListMesh *mesh) const {
    if (0 > i2 || i2 >= NumData()) {
        return 0;
    } else {
        return mSorts[mSortIndex]->GetListFromIdx(i2)->Mat(mesh);
    }
}

int NavListSortMgr::NumData() const { return mSorts[mSortIndex]->GetDataCount(); }

void NavListSortMgr::ClearIconLabels() {
    int numData = NumData();
    for (int i = 0; i < numData; i++) {
        mSorts[mSortIndex]->GetListFromIdx(i)->SetCollapseIconLabel(nullptr);
    }
}

Symbol NavListSortMgr::OnSelect(int i1) {
    if (i1 < 0 || i1 >= NumData()) {
        return Symbol(gNullStr);
    }
    return mSorts[mSortIndex]->GetListFromIdx(i1)->OnSelect();
}

Symbol NavListSortMgr::OnSelectDone(int i1) {
    if (i1 < 0 || i1 >= NumData()) {
        return Symbol(gNullStr);
    }
    return mSorts[mSortIndex]->GetListFromIdx(i1)->OnSelectDone();
}

void NavListSortMgr::OnHighlightChanged() {
    UnHighlightCurrent();
    if (!GetHighlightItem()) {
        MILO_ASSERT(GetHighlightItem(), 0x136);
    }
    GetHighlightItem()->OnHighlight();
}

bool NavListSortMgr::IsActive(int data) const {
    if (!NumData())
        return false;

    MILO_ASSERT((0) <= (data) && (data) < (NumData()), 0x79);

    if (data < 0 || data >= NumData())
        return false;

    return mSorts[mSortIndex]->GetListFromIdx(data)->IsActive();
}

Symbol NavListSortMgr::DataSymbol(int i1) const {
    NavListSortNode *pNode = mSorts[mSortIndex]->GetListFromIdx(i1);
    if (!pNode) {
        MILO_ASSERT(pNode, 0x9b);
    }
    return pNode->GetToken();
}

void NavListSortMgr::OnUnload() {
    FOREACH (it, mSorts) {
        (*it)->DeleteItemList();
        (*it)->DeleteTree();
    }
}

NavListSortNode *NavListSortMgr::GetHighlightItem() {
    return mSorts[mSortIndex]->GetHighlightItem();
}

void NavListSortMgr::StartPreview(int idx, TexMovie *tex) {
    if (idx >= 0) {
        if (idx < NumData()) {
            // auto something = mSorts[mSortIndex]->GetListFromIdx(i1)->GetToken();
            mSongPreview.Start(mSorts[mSortIndex]->GetListFromIdx(idx)->GetToken(), tex);
        }
    }
}

Symbol NavListSortMgr::OnGetToken(int idx) {
    if (idx < 0 || NumData() <= idx) {
        return Symbol(gNullStr);
    } else {
        return mSorts[mSortIndex]->GetListFromIdx(idx)->GetToken();
    }
}

int NavListSortMgr::DataIndex(Symbol s) const {
    MILO_NOTIFY_ONCE("DataIndex is not necessarily unique\n");
    auto node = mSorts[mSortIndex]->GetNode(s);
    if (!node) {
        return -1;
    } else {
        return node->GetIndex();
    }
}

Symbol NavListSortMgr::GetFirstChildSymbolFromHeaderSymbol(Symbol sym) {
    auto node = dynamic_cast<NavListHeaderNode *>(mSorts[mSortIndex]->GetNode(sym));
    if (!node) {
        return Symbol(gNullStr);
    } else {
        std::list<NavListSortNode *> c = node->GetChildren();
        return c.front()->GetToken();
    }
}

void NavListSortMgr::FinalizeHeaders() {
    if (mSortWithHeaders) {
        std::vector<int> tempVec(mSorts[mSortIndex]->GetDataCount(), 0);
        for (int i = 0; i < mHeaderIndicesAmongAll.size(); i++) {
            tempVec[mHeaderIndicesAmongAll[i]] = 1;
        }
        mHeaderIndicesAmongAll = tempVec;
    }
}

int NavListSortMgr::GetHeaderIndexFromListIndex(int idx) {
    Symbol token = OnGetToken(idx);
    int size = mHeaderIndicesAlone.size();
    for (int i = 0; i < size; i++) {
        if (token
            == mSorts[mSortIndex]->GetListFromIdx(mHeaderIndicesAlone[i])->GetToken()) {
            return i;
        }
    }
    return -1;
}

int NavListSortMgr::FirstDataIndex(Symbol s) {
    NavListSortNode *node = mSorts[mSortIndex]->GetNode(s);
    if (node) {
        return node->GetIndex();
    }
    return -1;
}

bool NavListSortMgr::IsDisabled(int i) {
    return mSorts[mSortIndex]->GetListFromIdx(i)->IsEnabled();
}

void NavListSortMgr::SetHighlightedIx(int i) {
    NavListSort *sort = mSorts[mSortIndex];
    sort->SetHighlightedIx(i);
}

int NavListSortMgr::GetHeaderCount() { return mHeaderIndicesAlone.size(); }

void NavListSortMgr::SortWithHeaders(int i) { mSortWithHeaders = i != 0; }

void NavListSortMgr::SetHeaderMode(int i) { mHeaderMode = i != 0; }

NavListSort *NavListSortMgr::GetCurrentSortHandle() {
    NavListSort *sort = mSorts[mSortIndex];
    if (!sort) {
        return nullptr;
    }
    return sort;
}

void NavListSortMgr::NextSort() { SetSort((mSortIndex + 1) % mSorts.size()); }

DataNode NavListSortMgr::OnCancel() { return DATA_UNHANDLED; }

BEGIN_HANDLERS(NavListSortMgr)
    HANDLE_EXPR(first_data_index, FirstDataIndex(_msg->Sym(2)))
    HANDLE_EXPR(is_active, IsActive(_msg->Int(2)))
    HANDLE_EXPR(is_disabled, IsDisabled(_msg->Int(2)))
    HANDLE_EXPR(on_select, OnSelect(_msg->Int(2)))
    HANDLE_EXPR(on_select_done, OnSelectDone(_msg->Int(2)))
    HANDLE_EXPR(on_cancel, OnCancel())
    HANDLE_EXPR(move_on, MoveOn())
    HANDLE_EXPR(clear_saved_highlight, mSavedHighlight = false)
    HANDLE_ACTION(set_highlighted_ix, SetHighlightedIx(_msg->Int(2)))
    HANDLE_EXPR(get_highlight_item, GetHighlightItem())
    HANDLE_ACTION(next_sort, NextSort())
    HANDLE_ACTION(set_sort_index, SetSort(_msg->Int(2)))
    HANDLE_ACTION(set_sort_name, SetSort(_msg->Sym(2)))
    HANDLE_EXPR(get_sort_index, mSortIndex)
    HANDLE_EXPR(get_current_sort_name, GetCurrentSortName())
    HANDLE_EXPR(get_current_sort, GetCurrentSortHandle())
    HANDLE_EXPR(are_headers_selectable, HeadersSelectable())
    HANDLE_EXPR(selection_is, SelectionIs(_msg->Sym(2)))
    HANDLE_EXPR(data_is, DataIs(_msg->Int(2), _msg->Sym(3)))
    HANDLE_ACTION(enter, OnEnter())
    HANDLE_ACTION(exit, OnExit())
    HANDLE_ACTION(unload, OnUnload())
    HANDLE_ACTION(start_preview, StartPreview(_msg->Int(2), _msg->Obj<TexMovie>(3)))
    HANDLE_ACTION(stop_preview, StopPreview())
    HANDLE_EXPR(get_token, OnGetToken(_msg->Int(2)))
    HANDLE_ACTION(set_header_mode, SetHeaderMode(_msg->Int(2)))
    HANDLE_EXPR(get_header_mode, mHeaderMode)
    HANDLE_EXPR(entering_header_mode, mEnteringHeaderMode)
    HANDLE_EXPR(exiting_header_mode, mExitingHeaderMode)
    HANDLE_ACTION(sort_with_headers, SortWithHeaders(_msg->Int(2)))
    HANDLE_EXPR(is_data_header, IsHeader(_msg->Int(2)))
    HANDLE_EXPR(
        get_header_symbol_from_child_symbol, GetHeaderSymbolFromChildSymbol(_msg->Sym(2))
    )
    HANDLE_EXPR(get_header_count, GetHeaderCount())
    HANDLE_EXPR(
        get_header_index_from_list_index, GetHeaderIndexFromListIndex(_msg->Int(2))
    )

    HANDLE_EXPR(
        get_list_index_from_header_index, GetListIndexFromHeaderIndex(_msg->Int(2))
    )
    HANDLE_EXPR(
        get_header_index_from_child_list_index,
        GetHeaderIndexFromChildListIndex(_msg->Int(2))
    )
    HANDLE_ACTION(do_uncollapse, DoUncollapse())
    HANDLE_EXPR(
        get_first_child_symbol_from_header_symbol,
        GetFirstChildSymbolFromHeaderSymbol(_msg->Sym(2))
    )
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
