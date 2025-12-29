#include "SongSortMgr.h"

#include "lazer/game/GameMode.h"
#include "SongSort.h"
#include "SongSortByDiff.h"
#include "SongSortNode.h"
#include "ui/UI.h"
#include "lazer/meta_ham/MetaPerformer.h"

BEGIN_HANDLERS(SongSortMgr)

END_HANDLERS

SongSortByDiff::SongSortByDiff() {
    static Symbol by_difficulty("by_difficulty");
    unk58 = by_difficulty.Str();
}

void SongSortMgr::Init(SongPreview &preview) {
    MILO_ASSERT(!TheSongSortMgr, 0x21);
    TheSongSortMgr = new SongSortMgr(preview);
    TheContentMgr.RegisterCallback(TheSongSortMgr, false);
}

void SongSortMgr::Terminate() {
    TheContentMgr.UnregisterCallback(TheSongSortMgr, false);
    MILO_ASSERT(TheSongSortMgr, 0x2b);
    //TheSongSortMgr->something;
    TheSongSortMgr = nullptr;
}

bool SongSortMgr::SelectionIs(Symbol sym) {
    static Symbol song("song");
    static Symbol header("header");
    static Symbol function("function");
    NavListSortNode *node;
    if (sym == song) {
        return dynamic_cast<SongSortNode *>(GetHighlightItem()) == 0;
    }
    if (sym == header) {
        return dynamic_cast<SongHeaderNode *>(GetHighlightItem()) == 0;
    }
    else {
        if (sym != function) {
            return false;
        }
        //return dynamic_cast<SongFunctionNode>(GetHighlightItem());
    }
    return dynamic_cast<NavListSortNode *>(GetHighlightItem()) == 0;
}

void SongSortMgr::OnHighlightChanged() {
    MILO_ASSERT(GetHighlightItem(), 0x75);

    if (GetHighlightItem()->OnSelectDone() /*fix this shxt*/) {
        if (!TheUI->GetTransitionState()) {
            static Symbol refresh_selected_song("refresh_selected_song");
        }
        // message::message something
    }
}

void SongSortMgr::MarkElementsProvided(UIListProvider *prov) {
    if (prov) {
        for (int i = 0; i < prov->NumData(); i++) {
            Symbol sym = prov->DataSymbol(i);
            SongSortNode *ssn = dynamic_cast<SongSortNode *>(GetCurrentSort()->GetNode(sym));
            if (ssn) {
                ssn->SetInPlaylist(true);
            }
        }
    }
}

void SongSortMgr::MarkElementInPlaylist(Symbol sym, bool b) {
    NavListSort *sort = GetCurrentSort();
    SongSortNode *ssn = dynamic_cast<SongSortNode *>(sort->GetNode(sym));
    if (ssn) {
        ssn->SetInPlaylist(b);
    }
}

int SongSortMgr::GetListIndexFromHeaderIndex(int i1) {
    int size = GetHeadersB().size();
    if (i1 < 0) {
        if (size > i1) {
            return GetHeadersBAtIdx(0);
        }
    }
    else {
        if (i1 < size) {
            return GetHeadersBAtIdx(i1);
        }
        if (0 < size) {
            return GetHeadersBAtIdx(size - 1);
        }
    }
    return 1;
}

void SongSortMgr::OnSetlistChanged() {
    Sorts()[GetCurrentSortIdx()]->BuildItemList();
    Sorts()[GetCurrentSortIdx()]->BuildTree();
    static Symbol refresh_setlist("refresh_setlist");
}

Symbol SongSortMgr::DetermineHeaderSymbolForSong(Symbol sym) {
    return static_cast<SongSort *>(GetCurrentSort())->DetermineHeaderSymbolFromSong(sym);
}

void SongSortMgr::SetQuasiRandomSong() {
    int numIndices = unk94.size();
    MILO_ASSERT(numIndices > 0, 0x175);

    int random = rand();
    int uVar8 = (unk94.end() - unk94.begin() >> 3) + (numIndices < 0 && (numIndices & 1) != 0);
    int iVar2 = (random - (random / uVar8) * uVar8) * 4;
    int iVar1 = unk94[iVar2];
    unk94.push_back(iVar1);
    auto piVar3 = mSorts[mCurrentSortIdx]->DataSymbol(iVar1 * 4);
    auto ass = MetaPerformer::Current();
    //auto puVar7 = ass->SelectSong(piVar3, iVar1);
    ass->SetSong(piVar3);
}

bool SongSortMgr::HeadersSelectable() {
    static Symbol song_select_mode("song_select_mode");
    static Symbol song_select_story("song_select_story");
    static Symbol song_select_practice("song_select_practice");
    Symbol mode = TheGameMode->Property(song_select_mode, true)->Sym();
    if (song_select_story != mode) {
        mode = TheGameMode->Property(song_select_mode, true)->Sym();
        if (song_select_practice != mode) {
            return true;
        }
    }
    return false;
}

bool SongSortMgr::DataIs(int i1, Symbol sym) {
    static Symbol song("song");
    static Symbol header("header");
    static Symbol function("function");

    if (sym == song) {
        return dynamic_cast<SongSortNode *>(mSorts[mCurrentSortIdx]->GetListFromIdx(i1)) == 0;
    }
    if (sym == header) {
        return dynamic_cast<SongHeaderNode *>(mSorts[mCurrentSortIdx]->GetListFromIdx(i1)) == 0;
    }
    else {
        if (sym == function) {
            return false;
        }
        //return dynamic_cast<SongFunctionNode *>(mSorts[mCurrentSortIdx]->GetListFromIdx(i1)) == 0;
    }
    return mSorts[mCurrentSortIdx]->GetListFromIdx(i1) == 0;
}

int SongSortMgr::FirstArtistSongIndex(Symbol sym) {

    return 0;
}