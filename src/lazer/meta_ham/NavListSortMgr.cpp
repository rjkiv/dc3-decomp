#include "NavListSortMgr.h"
#include "meta/SongPreview.h"
#include "meta_ham/NavListNode.h"
#include "os/System.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

// NavListSortMgr TheNavListSortMgr;

NavListSortMgr::NavListSortMgr(SongPreview &songprev)
    : mSongPreview(&songprev), mHeaderMode(0), mEnteringHeaderMode(0),
      mExitingHeaderMode(0), unk60(0), unk6c(0), unk70(0) {};

void NavListSortMgr::SetSort(int i1) {}

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

void NavListSortMgr::SetHeaderUncollapsed(Symbol sym) {}
