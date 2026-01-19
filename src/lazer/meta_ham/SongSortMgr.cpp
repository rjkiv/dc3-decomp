#include "SongSortMgr.h"

#include "HamSongMgr.h"
#include "NavListSortMgr.h"
#include "SongSortBySong.h"
#include "lazer/game/GameMode.h"
#include "SongSort.h"
#include "SongSortByDiff.h"
#include "SongSortByLocation.h"
#include "SongSortNode.h"
#include "meta/SongPreview.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UI.h"
#include "lazer/meta_ham/MetaPerformer.h"
#include "ProfileMgr.h"
#include "ui/UIListProvider.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
BEGIN_HANDLERS(SongSortMgr)
    HANDLE_ACTION(get_setlist_mode, 0)
    HANDLE_ACTION(set_setlist_mode, SetSetlistMode(_msg->Int(2) != 0))
    HANDLE_ACTION(
        mark_elements_provided, MarkElementsProvided(_msg->Obj<UIListProvider>(2))
    )
    HANDLE_ACTION(
        mark_element_in_playlist, MarkElementInPlaylist(_msg->Sym(2), _msg->Int(3) != 0)
    )
    HANDLE_ACTION(setup_quasi_random_songs, SetupQuasiRandomSongs())
    HANDLE_ACTION(set_quasi_random_song, SetQuasiRandomSong())
    HANDLE_EXPR(first_artist_song_index, FirstArtistSongIndex(_msg->Sym(2)))
    HANDLE_EXPR(
        determine_header_symbol_for_song, DetermineHeaderSymbolForSong(_msg->Sym(2))
    )
    HANDLE_SUPERCLASS(NavListSortMgr)
END_HANDLERS

SongSort::SongSort() {}

SongSortMgr::SongSortMgr(SongPreview &sp) : NavListSortMgr(sp) {
    SetName("song_offer_provider", ObjectDir::Main());
    mSorts.push_back(new SongSortByDiff());
    mSorts.push_back(new SongSortBySong());
    mSorts.push_back(new SongSortByLocation());
    unk90 = 0;
}

SongSortMgr::~SongSortMgr() {}

SongSortByDiff::SongSortByDiff() {
    static Symbol by_difficulty("by_difficulty");
    SetSortName(by_difficulty);
}

SongSortByLocation::SongSortByLocation() {
    static Symbol by_location("by_location");
    SetSortName(by_location);
}

void SongSortMgr::Init(SongPreview &preview) {
    MILO_ASSERT(!TheSongSortMgr, 0x21);
    TheSongSortMgr = new SongSortMgr(preview);
    TheContentMgr.RegisterCallback(TheSongSortMgr, false);
}

void SongSortMgr::Terminate() {
    TheContentMgr.UnregisterCallback(TheSongSortMgr, false);
    MILO_ASSERT(TheSongSortMgr, 0x2b);
    if (!TheSongSortMgr) {
        TheSongSortMgr = nullptr;
        return;
    }
    RELEASE(TheSongSortMgr);
}

bool SongSortMgr::SelectionIs(Symbol selection) {
    static Symbol song("song");
    static Symbol header("header");
    static Symbol function("function");
    if (selection == song) {
        return dynamic_cast<SongSortNode *>(GetHighlightItem()) != nullptr;
    } else if (selection == header) {
        return dynamic_cast<SongHeaderNode *>(GetHighlightItem()) != nullptr;
    } else if (selection == function) {
        return dynamic_cast<SongFunctionNode *>(GetHighlightItem()) != nullptr;
    }
    return false;
}

void SongSortMgr::OnHighlightChanged() {
    this->UnHighlightCurrent();
    MILO_ASSERT(GetHighlightItem(), 0x75);
    GetHighlightItem()->OnHighlight();
    if (!TheUI->GetTransitionState()) {
        static Symbol refresh_selected_song("refresh_selected_song");
        static Message msg(refresh_selected_song);
        TheUI->Handle(msg, false);
    }
}

void SongSortMgr::MarkElementsProvided(UIListProvider *prov) {
    if (prov) {
        for (int i = 0; i < prov->NumData(); i++) {
            Symbol sym = prov->DataSymbol(i);
            SongSortNode *ssn =
                dynamic_cast<SongSortNode *>(GetCurrentSort()->GetNode(sym));
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
    int size = mHeadersB.size();
    if (i1 < 0 && 0 < size) {
        return mHeadersB.front();
    }
    if (i1 < size) {
        return mHeadersB[size - 1];
    }

    return 1;
}

void SongSortMgr::OnSetlistChanged() {
    mSorts[mCurrentSortIdx]->BuildItemList();
    mSorts[mCurrentSortIdx]->UpdateHighlight();
    static Symbol refresh_setlist("refresh_setlist");
    static Message refresh_setlist_msg(refresh_setlist);
    TheUI->Handle(refresh_setlist_msg, false);
}

void SongSortMgr::OnSetlistModeChanged() {
    mSorts[mCurrentSortIdx]->BuildItemList();
    mSorts[mCurrentSortIdx]->UpdateHighlight();

    static Symbol on_change_setlist_mode("on_change_setlist_mode");
    static Message on_change_setlist_mode_msg(on_change_setlist_mode);
    TheUI->Handle(on_change_setlist_mode_msg, false);

    static Symbol update_held_buttons("update_held_buttons");
    static Message update_held_buttons_msg(update_held_buttons);
    TheUI->Handle(update_held_buttons_msg, false);
}

Symbol SongSortMgr::DetermineHeaderSymbolForSong(Symbol sym) {
    return static_cast<SongSort *>(GetCurrentSort())->DetermineHeaderSymbolFromSong(sym);
}

void SongSortMgr::SetQuasiRandomSong() {
    int numIndices = unk94.size();
    MILO_ASSERT(numIndices > 0, 0x175);

    int random = rand();
    int uVar8 =
        (unk94.end() - unk94.begin() >> 3) + (numIndices < 0 && (numIndices & 1) != 0);
    int iVar2 = (random - (random / uVar8) * uVar8) * 4;
    int iVar1 = unk94[iVar2];
    unk94.push_back(iVar1);
    auto piVar3 = mSorts[mCurrentSortIdx]->DataSymbol(iVar1 * 4);
    auto ass = MetaPerformer::Current();
    // auto puVar7 = ass->SelectSong(piVar3, iVar1);
    ass->SetSong(piVar3);
}

bool SongSortMgr::HeadersSelectable() {
    static Symbol song_select_mode("song_select_mode");
    static Symbol song_select_story("song_select_story");
    static Symbol song_select_practice("song_select_practice");
    Symbol mode;
    mode = TheGameMode->Property(song_select_mode, true)->Sym();
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
        return dynamic_cast<SongSortNode *>(mSorts[mCurrentSortIdx]->GetListFromIdx(i1))
            == 0;
    }
    if (sym == header) {
        return dynamic_cast<SongHeaderNode *>(mSorts[mCurrentSortIdx]->GetListFromIdx(i1))
            == 0;
    } else {
        if (sym == function) {
            return false;
        }
        // return dynamic_cast<SongFunctionNode
        // *>(mSorts[mCurrentSortIdx]->GetListFromIdx(i1)) == 0;
    }
    return mSorts[mCurrentSortIdx]->GetListFromIdx(i1) == 0;
}

int SongSortMgr::FirstArtistSongIndex(Symbol sym) {
    int dataCount = mSorts[mCurrentSortIdx]->GetDataCount();
    for (int i = 0; i < dataCount; i++) {
        SongSortNode *ssNode =
            dynamic_cast<SongSortNode *>(mSorts[mCurrentSortIdx]->GetListFromIdx(i));
        if (ssNode) {
            Symbol artist = ssNode->GetArtist();
            if (sym == artist) {
                int idx = GetHeaderIndexFromChildListIndex(i);
                return idx;
            }
        }
    }
    return 0;
}

void SongSortMgr::RebuildSongRecordMap() {
    unk78.clear();
    std::vector<int> rankedSongs;
    TheHamSongMgr.GetRankedSongs(rankedSongs);
    unk90 = rankedSongs.size();
    for (int i = 0; i < rankedSongs.size(); i++) {
        const HamSongMetadata *metadata = TheHamSongMgr.Data(i);
        if ((metadata && !metadata->IsFake())
            && TheProfileMgr.IsContentUnlocked(metadata->ShortName())) {
            // auto first = metadata->DefaultCharacter();
            SongRecord second = SongRecord(metadata);
            // std::pair<Symbol, SongRecord> p;
            unk78.insert(std::pair<Symbol, SongRecord>(metadata->ShortName(), second));
        }
    }
}

Symbol SongSortMgr::MoveOn() {
    static Symbol song_select_quickplay("song_select_quickplay");
    static Symbol song_select_story("song_select_story");
    static Symbol song_select_practice("song_select_practice");
    static Symbol song_select_jukebox("song_select_jukebox");

    Symbol mode = TheGameMode->Property("song_select_mode", true)->Sym();
    if (song_select_quickplay == mode) {
        static Symbol move_on_quickplay("move_on_quickplay");
        UIPanel *songSelectPanel =
            ObjectDir::Main()->Find<UIPanel>("song_select_panel", true);
        static Message move_on_quickplay_msg("move_on_quickplay");
        songSelectPanel->HandleType(move_on_quickplay_msg);
        return gNullStr;
    } else {
        if (song_select_story == mode || song_select_practice == mode
            || mode == song_select_jukebox) {
            Symbol ready_screen("ready_screen");
            const DataNode *prop = TheGameMode->Property(ready_screen);
            return prop->Sym();
        }
        MILO_FAIL("Unknown song_select_mode\n");
    }
    return gNullStr;
}

void SongSortMgr::OnEnter() {
    MetaPerformer::Current()->ResetSongs();
    static Symbol bass("bass");
    static Symbol guitar("guitar");
    static Symbol band("band");
    RebuildSongRecordMap();
    FOREACH (it, mSorts) {
        (*it)->BuildTree();
    }
    auto current = mSorts[mCurrentSortIdx];
    current->BuildItemList();
    if (unk48) {
        current->SetHighlightID(unk44);
        unk48 = false;
    }
    current->UpdateHighlight();
}
