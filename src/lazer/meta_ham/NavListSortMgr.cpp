#include "NavListSortMgr.h"

#include "NavListSort.h"
#include "meta/SongPreview.h"
#include "meta_ham/NavListNode.h"
#include "os/System.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

// NavListSortMgr TheNavListSortMgr;

BEGIN_HANDLERS(NavListSortMgr)
    HANDLE_EXPR(first_data_index, unk34.front()->GetNode(_msg->Sym(2)))
    HANDLE_EXPR(is_active, IsActive(_msg->Int(2)))
    HANDLE_EXPR(is_disabled, !IsActive(_msg->Int(2)))
    HANDLE_EXPR(on_select, OnSelect(_msg->Int(2)))
    HANDLE_EXPR(on_select_done, OnSelectDone(_msg->Int(2)))
    //on_cancel, move_on here
    HANDLE_EXPR(clear_saved_highlight, unk48 = _msg->Int(0))
    //set_highlighted_ix
    HANDLE_ACTION(get_highlight_item, GetHighlightItem())
    HANDLE_ACTION(next_sort, SetSort(_msg->Sym(2)))
    HANDLE_ACTION(set_sort_index, SetSort(_msg->Int(2)))
    HANDLE_ACTION(set_sort_name, SetSort(_msg->Sym(2)))
    //get_sort_index
    HANDLE_ACTION(get_current_sort_name, GetCurrentSortName())
    HANDLE_ACTION(get_current_sort, GetCurrentSort())
    HANDLE_EXPR(are_headers_selectable, unk6c)
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
    HANDLE_EXPR(is_data_header, unk60[_msg->Int(2)])
    HANDLE_ACTION(get_header_symbol_from_child_symbol, GetHeaderSymbolFromChildSymbol(_msg->Sym(2)))
    HANDLE_ACTION(get_header_count, unk60.size())
    HANDLE_ACTION(get_header_index_from_list_index, GetHeaderIndexFromListIndex(_msg->Int(2)))
    HANDLE_ACTION(get_list_index_from_header_index, GetListIndexFromHeaderIndex(_msg->Int(2)))
    HANDLE_ACTION(get_header_index_from_child_list_index, GetHeaderIndexFromChildListIndex(_msg->Int(2)))
    HANDLE_ACTION(do_uncollapse, DoUncollapse())
    HANDLE_ACTION(get_first_child_symbol_from_header_symbol, _msg->Sym(2))
    HANDLE_SUPERCLASS(UIListProvider)

    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

NavListSortMgr::NavListSortMgr(SongPreview &songprev)
    : mSongPreview(&songprev), mHeaderMode(0), mEnteringHeaderMode(0),
      mExitingHeaderMode(0), unk60(0), unk6c(0), unk70(0) {};

void NavListSortMgr::StopPreview() { mSongPreview->Start(gNullStr, 0); }

void NavListSortMgr::SetHeaderMode(bool b) { mHeaderMode = b; }

void NavListSortMgr::SetEnteringHeaderMode(bool b) { mEnteringHeaderMode = b; }

void NavListSortMgr::SetExitingHeaderMode(bool b) { mExitingHeaderMode = b; }

void NavListSortMgr::AddHeaderIndex(int idx) {
    if (unk6c) {
        unk54.push_back(idx);
        unk60.push_back(idx);
    }
}

bool NavListSortMgr::IsHeader(int idx) {
    if (0 <= idx && unk54.size() > idx) {
        return unk54[idx] != false;
    }
    return false;
}

void NavListSortMgr::ClearHeaders() {
    unk54.clear();
    unk60.clear();
}
