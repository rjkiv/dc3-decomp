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
    : mSongPreview(&songprev), mCurrentSortIdx(0), unk48(false), mHeaderMode(false),
      mEnteringHeaderMode(false), mExitingHeaderMode(false), mHeadersSelectable(false) {
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
    NavListSort *sort = mSorts[mCurrentSortIdx];
    if (!sort->GetNode(sym)) {
        return gNullStr;
    } else {
        NavListHeaderNode *node =
            dynamic_cast<NavListHeaderNode *>(sort->GetNode(sym)->Parent());
        if (!node) {
            return sort->GetSortName(); // unsure
        } else {
            return node->GetToken();
        }
    }
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
            return false;
        }
    }
    return true;
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
        mSorts[mCurrentSortIdx]->GetUnk54()->OnUnHighlight();
        mSorts[mCurrentSortIdx]->SetUnk54(0);
    }
}

void NavListSortMgr::ContentMounted(const char *c1, const char *c2) {
    NavListSortNode *node = mSorts[mCurrentSortIdx]->GetUnk50();
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
    NavListSortNode *node = GetHighlightItem();
    if (node) {
        node->GetID(unk44);
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
    for (int i = NumData(); i != 0; i--) {
        mSorts[mCurrentSortIdx]->GetListFromIdx(i)->SetCollapseIconLabel(nullptr);
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

void NavListSortMgr::StartPreview(int idx, TexMovie *tex) {
    if (idx >= 0) {
        if (idx < NumData()) {
            // auto something = mSorts[mCurrentSortIdx]->GetListFromIdx(i1)->GetToken();
            mSongPreview->Start(
                mSorts[mCurrentSortIdx]->GetListFromIdx(idx)->GetToken(), tex
            );
        }
    }
}

Symbol NavListSortMgr::OnGetToken(int idx) {
    if (idx < 0 || NumData() <= idx) {
        return Symbol(gNullStr);
    } else {
        return mSorts[mCurrentSortIdx]->GetListFromIdx(idx)->GetToken();
    }
}

int NavListSortMgr::DataIndex(Symbol s) const {
    static std::list<String> strings;
    // const char *str = "DataIndex is not necessarily unique\n";
    bool added = AddToStrings("DataIndex is not necessarily unique\n", strings);
    if (added)
        MILO_NOTIFY("DataIndex is not necessarily unique\n");
    auto node = mSorts[mCurrentSortIdx]->GetNode(s);
    if (!node) {
        return -1;
    } else {
        return node->GetStartIx();
    }
}

Symbol NavListSortMgr::GetFirstChildSymbolFromHeaderSymbol(Symbol sym) {
    auto node = dynamic_cast<NavListHeaderNode *>(mSorts[mCurrentSortIdx]->GetNode(sym));
    if (!node) {
        return Symbol(gNullStr);
    } else {
        std::list<NavListSortNode *> c = node->Children();
        return c.front()->GetToken();
    }
}

void NavListSortMgr::FinalizeHeaders() {
    if (mHeadersSelectable) {
        std::vector<int> tempVec(mSorts[mCurrentSortIdx]->GetDataCount(), 0);
        for (int i = 0; i < mHeadersA.size(); i++) {
            tempVec[mHeadersA[i]] = 1;
        }
        mHeadersA = tempVec;
    }
}

int NavListSortMgr::GetHeaderIndexFromListIndex(int idx) {
    Symbol token = OnGetToken(idx);
    for (int i = 0; i < mHeadersB.size(); i++) {
        if (token == mSorts[mCurrentSortIdx]->GetListFromIdx(mHeadersB[i])->GetToken()) {
            return i;
        }
    }
    return -1;
}

int NavListSortMgr::FirstDataIndex(Symbol s) {
    NavListSortNode *node = mSorts[mCurrentSortIdx]->GetNode(s);
    if (node) {
        return node->GetStartIx();
    }
    return -1;
}

bool NavListSortMgr::IsDisabled(int i) {
    return mSorts[mCurrentSortIdx]->GetListFromIdx(i)->IsActive();
}

void NavListSortMgr::SetHighlightedIx(int i) {
    NavListSort *sort = mSorts[mCurrentSortIdx];
    sort->SetHighlightedIx(i);
}

int NavListSortMgr::GetHeaderCount() { return mHeadersB.size(); }

void NavListSortMgr::SortWithHeaders(int i) { mHeadersSelectable = i != 0; }

void NavListSortMgr::SetHeaderMode(int i) { mHeaderMode = i != 0; }

NavListSort *NavListSortMgr::GetCurrentSortHandle() {
    NavListSort *sort = mSorts[mCurrentSortIdx];
    if (!sort) {
        return nullptr;
    }
    return sort;
}

void NavListSortMgr::NextSort() { SetSort((mCurrentSortIdx + 1) % mSorts.size()); }

DataNode NavListSortMgr::OnCancel() { return DATA_UNHANDLED; }

BEGIN_HANDLERS(NavListSortMgr)
    HANDLE_EXPR(first_data_index, FirstDataIndex(_msg->Sym(2)))
    HANDLE_EXPR(is_active, IsActive(_msg->Int(2)))
    HANDLE_EXPR(is_disabled, IsDisabled(_msg->Int(2)))
    HANDLE_EXPR(on_select, OnSelect(_msg->Int(2)))
    HANDLE_EXPR(on_select_done, OnSelectDone(_msg->Int(2)))
    HANDLE_EXPR(on_cancel, OnCancel())
    HANDLE_EXPR(move_on, MoveOn())
    HANDLE_EXPR(clear_saved_highlight, unk48 = false)
    HANDLE_ACTION(set_highlighted_ix, SetHighlightedIx(_msg->Int(2)))
    HANDLE_EXPR(get_highlight_item, GetHighlightItem())
    HANDLE_ACTION(next_sort, NextSort())
    HANDLE_ACTION(set_sort_index, SetSort(_msg->Int(2)))
    HANDLE_ACTION(set_sort_name, SetSort(_msg->Sym(2)))
    HANDLE_EXPR(get_sort_index, mCurrentSortIdx)
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
