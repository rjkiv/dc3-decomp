#include "SongSortMgr.h"

#include "HamSongMgr.h"
#include "lazer/game/GameMode.h"
#include "SongSort.h"
#include "SongSortByDiff.h"
#include "SongSortByLocation.h"
#include "SongSortNode.h"
#include "ui/UI.h"
#include "lazer/meta_ham/MetaPerformer.h"
#include "ProfileMgr.h"
#include "ui/UIPanel.h"
BEGIN_HANDLERS(SongSortMgr)

END_HANDLERS

SongSort::SongSort() {  }

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
    //TheSongSortMgr->Handle(null, 1); // cant figure out what function is getting called here
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
    return GetHighlightItem() == 0;
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
    int size = mHeadersB.size();
    if (i1 < 0 && 0 < size) {
        return mHeadersB.front();
    }
    if (i1 >= size) {
        return mHeadersB[i1];
    }
    if (size > 0) {
        return mHeadersB[size - 1];
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
    int dataCount = mSorts[mCurrentSortIdx]->GetDataCount();
    for (int i = 0; i < dataCount; i++) {
        SongSortNode * ssNode = dynamic_cast<SongSortNode *>(mSorts[mCurrentSortIdx]->GetListFromIdx(i));
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
        if ((metadata && !metadata->IsFake()) && TheProfileMgr.IsContentUnlocked(metadata->ShortName())) {

            //auto first = metadata->DefaultCharacter();
            SongRecord second = SongRecord(metadata);
            //std::pair<Symbol, SongRecord> p;
            unk78.insert(std::pair<Symbol, SongRecord>(metadata->ShortName(), second));
        }
    }
}

Symbol SongSortMgr::MoveOn() {
    static Symbol song_select_quickplay("song_select_quickplay");
    static Symbol song_select_story("song_select_story");
    static Symbol song_select_practice("song_select_practice");
    static Symbol song_select_jukebox("song_select_jukebox");

    auto mode = TheGameMode->Property("song_select_mode", true)->Sym();
    if (song_select_quickplay == mode) {
        static Message move_on_quickplay("move_on_quickplay", 0);
        auto panel = ObjectDir::Main()->Find<UIPanel>("song_select_panel", true);
        //HandleType(move_on_quickplay);
        panel->HandleType(move_on_quickplay);
    }
    else {
        if (song_select_story == mode || song_select_practice == mode || mode == song_select_jukebox) {
            return TheGameMode->Property("ready_screen", true)->Sym();
        }
        FormatString fs = "Unknown song_select_mode\n";
        TheDebug.Fail(fs.Str(), 0);

    }
    return 0;
}

void SongSortMgr::OnEnter() {
    MetaPerformer::Current()->ResetSongs();
    static Symbol bass("bass");
    static Symbol guitar("guitar");
    static Symbol band("band");
    RebuildSongRecordMap();
    FOREACH(it, mSorts) {
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