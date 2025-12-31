#include "NavListSortMgr.h"

#include "NavListSort.h"
#include "macros.h"
#include "meta/SongPreview.h"
#include "meta_ham/NavListNode.h"
#include "obj/Data.h"
#include "os/System.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

BEGIN_HANDLERS(NavListSortMgr)
    HANDLE_EXPR(first_data_index, mSorts.front()->GetNode(_msg->Sym(2)))
    HANDLE_EXPR(is_active, IsActive(_msg->Int(2)))
    HANDLE_EXPR(is_disabled, !IsActive(_msg->Int(2)))
    HANDLE_EXPR(on_select, OnSelect(_msg->Int(2)))
    HANDLE_EXPR(on_select_done, OnSelectDone(_msg->Int(2)))
    // on_cancel, move_on here
    HANDLE_EXPR(clear_saved_highlight, unk48 = _msg->Int(0))
    // set_highlighted_ix
    HANDLE_ACTION(get_highlight_item, GetHighlightItem())
    HANDLE_ACTION(next_sort, SetSort(_msg->Sym(2)))
    HANDLE_ACTION(set_sort_index, SetSort(_msg->Int(2)))
    HANDLE_ACTION(set_sort_name, SetSort(_msg->Sym(2)))
    HANDLE_EXPR(get_sort_index, mCurrentSortIdx)
    HANDLE_ACTION(get_current_sort_name, GetCurrentSortName())
    HANDLE_ACTION(get_current_sort, GetCurrentSort())
    HANDLE_EXPR(are_headers_selectable, mHeadersSelectable)
    HANDLE_EXPR(selection_is, _msg->Sym(2))
    HANDLE_EXPR(data_is, _msg->Sym(3))
    HANDLE_ACTION(enter, _msg->Sym(2))
    HANDLE_ACTION(exit, OnExit())
    HANDLE_ACTION(unload, OnUnload())
    HANDLE_ACTION(start_preview, StartPreview(_msg->Int(2), _msg->Obj<TexMovie>(3)))
    HANDLE_ACTION(stop_preview, StopPreview())
    HANDLE_ACTION(get_token, OnGetToken(_msg->Int(2)))
    HANDLE_EXPR(set_header_mode, mHeaderMode = _msg->Int(2))
    HANDLE_EXPR(get_header_mode, mHeaderMode)
    HANDLE_EXPR(entering_header_mode, mEnteringHeaderMode)
    HANDLE_EXPR(exiting_header_mode, mExitingHeaderMode)
    HANDLE_EXPR(sort_with_headers, _msg->Int(2))
    HANDLE_EXPR(is_data_header, mHeadersB[_msg->Int(2)])
    HANDLE_ACTION(
        get_header_symbol_from_child_symbol, GetHeaderSymbolFromChildSymbol(_msg->Sym(2))
    )
    HANDLE_ACTION(get_header_count, mHeadersB.size())
    HANDLE_ACTION(
        get_header_index_from_list_index, GetHeaderIndexFromListIndex(_msg->Int(2))
    )
    HANDLE_ACTION(
        get_list_index_from_header_index, GetListIndexFromHeaderIndex(_msg->Int(2))
    )
    HANDLE_ACTION(
        get_header_index_from_child_list_index,
        GetHeaderIndexFromChildListIndex(_msg->Int(2))
    )
    HANDLE_ACTION(do_uncollapse, DoUncollapse())
    HANDLE_ACTION(get_first_child_symbol_from_header_symbol, _msg->Sym(2))
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

NavListSortMgr::NavListSortMgr(SongPreview &songprev)
    : mSongPreview(&songprev), mCurrentSortIdx(0), unk48(false), mHeaderMode(0),
      mEnteringHeaderMode(0), mExitingHeaderMode(0), mHeadersSelectable(0), unk70(0) {
    unk44 = new DataArray(0);
};

NavListSortMgr::~NavListSortMgr() {
    DeleteAll(mSorts);
    unk44->Release();
}

void NavListSortMgr::StopPreview() { mSongPreview->Start(gNullStr, 0); }

void NavListSortMgr::SetHeaderMode(bool b) { mHeaderMode = b; }

void NavListSortMgr::SetEnteringHeaderMode(bool b) { mEnteringHeaderMode = b; }

void NavListSortMgr::SetExitingHeaderMode(bool b) { mExitingHeaderMode = b; }

void NavListSortMgr::AddHeaderIndex(int idx) {
    if (mHeadersSelectable) {
        mHeadersA.push_back(idx);
        mHeadersB.push_back(idx);
    }
}

bool NavListSortMgr::IsHeader(int idx) {
    if (0 <= idx && mHeadersA.size() > idx) {
        return mHeadersA[idx] != false;
    }
    return false;
}

void NavListSortMgr::ClearHeaders() {
    mHeadersA.clear();
    mHeadersB.clear();
}

Symbol NavListSortMgr::GetHeaderSymbolFromChildSymbol(Symbol sym) {
    auto a = mSorts[mCurrentSortIdx];
    if (!a->GetNode(sym.Str())) {
        sym = gNullStr;
    }
    return sym;
}

NavListSort *NavListSortMgr::GetCurrentSort() { return mSorts[mCurrentSortIdx]; }

Symbol NavListSortMgr::GetCurrentSortName() {
    NavListSort *pCurrentSort = mSorts[mCurrentSortIdx];
    if (!pCurrentSort) {
        MILO_ASSERT(pCurrentSort, 0x19c);
    }
    return pCurrentSort->GetSortName();
}

void NavListSortMgr::SetSort(int idx) {
    if (idx >= 0 && idx < mSorts.size()) {
        unk70.clear();
        ClearIconLabels();
        mCurrentSortIdx = idx;
        mSorts[idx]->BuildItemList();
        mSorts[mCurrentSortIdx]->SetHighlightedIx(0);
        mSorts[mCurrentSortIdx]->UpdateHighlight();
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
    FOREACH (it, unk70) {
        if (*it == sym) {
            return;
        }
    }
    unk70.push_back(sym);
}

void NavListSortMgr::SetHeaderCollapsed(Symbol sym) {
    FOREACH (it, unk70) {
        if (*it == sym) {
            unk70.erase(it);
            return;
        }
    }
}

bool NavListSortMgr::IsHeaderCollapsed(Symbol sym) {
    FOREACH (it, unk70) {
        if (*it == sym) {
            break;
        }
    }
    return false;
}

bool NavListSortMgr::IsIndexHeader(int idx) {
    if (idx >= 0 && mHeadersA.size() >= idx) {
        return mHeadersA[idx] != 0;
    }
    return false;
}

void NavListSortMgr::Text(int i1, int i2, UIListLabel *listlabel, UILabel *label) const {
    if (0 <= i2 && i2 < NumData()) {
        if (listlabel->Matches("header_collapse")) {
            mSorts[mCurrentSortIdx]->GetListFromIdx(i2)->SetCollapseIconLabel(label);
        }
        mSorts[mCurrentSortIdx]->GetListFromIdx(i2)->Text(listlabel, label);
    }
}

void NavListSortMgr::UnHighlightCurrent() {
    if (mSorts[mCurrentSortIdx]->GetUnk54()) {
        mSorts[mCurrentSortIdx]->SetHighlightItem(0);
        mSorts[mCurrentSortIdx]->SetUnk54(0);
    }
}

void NavListSortMgr::DoUncollapse() {
    if (!IsInHeaderMode()) {
        MILO_ASSERT(IsInHeaderMode(), 0x264);
    }
    mHeaderMode = false;
    mSorts.front()->SetHighlightItem(0);
    mSorts[mCurrentSortIdx]->BuildItemList();
}

UIComponent::State
NavListSortMgr::ComponentStateOverride(int i1, int i2, UIComponent::State state) const {
    if (!mSorts[mCurrentSortIdx]->GetListFromIdx(i2)->IsActive()) {
        return UIComponent::State::kDisabled;
    }
    return state;
}

int NavListSortMgr::GetListIndexFromHeaderIndex(int idx) {
    int size = mHeadersB.size();
    if (idx < 0) {
        if (0 < size) {
            return mHeadersB.front();
        }
    }
    if (idx < size) {
        return 0;
    }
    if (size > 0) {
        return mHeadersB[size - 1];
    }
    return 0;
}

void NavListSortMgr::OnExit() {
    if (GetHighlightItem()) {
        GetHighlightItem()->GetID(unk44);
        unk48 = true;
    }
}

void NavListSortMgr::Custom(int i1, int i2, UIListCustom *uilist, Hmx::Object *obj) const {
    if (0 <= i2 && i2 < NumData()) {
        mSorts[mCurrentSortIdx]->GetListFromIdx(i2)->Custom(uilist, obj);
    }
}

RndMat *NavListSortMgr::Mat(int i1, int i2, UIListMesh *mesh) const {
    if (0 > i2 || i2 >= NumData()) {
        return 0;
    } else {
        return mSorts[mCurrentSortIdx]->GetListFromIdx(i2)->Mat(mesh);
    }
}

int NavListSortMgr::NumData() const { return mSorts[mCurrentSortIdx]->GetDataCount(); }

void NavListSortMgr::ClearIconLabels() {
    for (int i = NumData(), j = 0; i != 0; i--, j++) {
        mSorts[mCurrentSortIdx]->GetListFromIdx(j)->SetCollapseIconLabel(0);
    }
}

Symbol NavListSortMgr::OnSelect(int i1) {
    if (i1 < 0 || i1 >= NumData()) {
        return Symbol(gNullStr);
    }
    return mSorts[mCurrentSortIdx]->GetListFromIdx(i1)->OnSelect();
}

Symbol NavListSortMgr::OnSelectDone(int i1) {
    if (i1 < 0 || i1 >= NumData()) {
        return Symbol(gNullStr);
    }
    return mSorts[mCurrentSortIdx]->GetListFromIdx(i1)->OnSelectDone();
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

    return mSorts[mCurrentSortIdx]->GetListFromIdx(data)->IsEnabled();
}

Symbol NavListSortMgr::DataSymbol(int i1) const {
    NavListSortNode *pNode = mSorts[mCurrentSortIdx]->GetListFromIdx(i1);
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
    return mSorts[mCurrentSortIdx]->GetUnk50();
}