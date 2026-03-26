#include "SongSort.h"

#include "AppLabel.h"
#include "ChallengeSort.h"
#include "SongRecord.h"
#include "SongSortMgr.h"
#include "game/GameMode.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/SongSortMgr.h"
#include "meta_ham/SongSortNode.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

SongSort::SongSort() {};

void SongSort::BuildTree() {};

void SongSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    TheSongSortMgr->ClearHeaders();
};

void SongSort::BuildItemList() {
    Symbol sym = gNullStr;
    if (unk50 && unk50->GetType() == kNodeFunction) {
        sym = unk50->GetToken();
    }
    DeleteItemList();
    static Symbol song_select_mode("song_select_mode");
    static Symbol song_select_story("song_select_story");
    static Symbol song_select_playlist("song_select_playlist");
    static Symbol random_song("random_song");
    static Symbol perform("perform");
    static Symbol dance_battle("dance_battle");
    bool inPerform = TheGameMode->InMode(perform, true);
    bool inDanceBattle = TheGameMode->InMode(dance_battle, true);
    Symbol prop;
    prop = TheGameMode->Property(song_select_mode, true)->Sym();
    bool check = prop == song_select_playlist;

    if (TheSongSortMgr->HeadersSelectable() && (inPerform || inDanceBattle) && !check) {
        static Symbol random_song("random_song");
        SongFunctionNode *node = new SongFunctionNode(
            nullptr, random_song, "ui/image/song_select_setlist_keep.png"
        );
        node->SetShortcut(unk30.front());
        unk3c.push_back(node);
        if (inPerform) {
            static Symbol playlists("playlists");
            SongFunctionNode *functionNode = new SongFunctionNode(
                nullptr, playlists, "ui/image/song_select_setlist_keep.png"
            );
            functionNode->SetShortcut(unk30.front());
            unk3c.push_back(functionNode);
        }
    } else if (check) {
        static Symbol finish_setlist("finish_setlist");
        SongFunctionNode *node = new SongFunctionNode(
            nullptr, finish_setlist, "ui/image/song_select_setlist_keep.png"
        );
        node->SetShortcut(unk30.front());
        unk3c.push_back(node);
    }

    FOREACH (it, unk3c) {
        (*it)->Renumber(mList);
    }

    FOREACH (it, unk30) {
        (*it)->Renumber(mList);
    }

    if (check) {
        FOREACH (it, unk3c) {
            (*it)->Renumber(mList);
        }
    }

    FOREACH (it, unk30) {
        (*it)->FinishBuildList(this);
    }

    if (sym.Str() != gNullStr) {
        unk50 = GetNode(sym);
    }

    TheSongSortMgr->FinalizeHeaders();
}

void SongSort::SetHighlightedIx(int i1) {
    unk54 = unk50;
    if (i1 >= 0 && i1 < mList.size()) {
        unk50 = mList[i1];
        TheSongSortMgr->OnHighlightChanged();
        return;
    }
    unk50 = 0;
};

void SongSort::SetHighlightItem(const NavListSortNode *node) {
    unk54 = unk50;
    unk50 = nullptr;
    if (node) {
        if (node->GetType() == kNodeFunction || node->GetType() == kNodeItem) {
            auto findIf = std::find_if(mList.begin(), mList.end(), SortNodeFind(node));
            if (findIf != mList.end()) {
                unk50 = *findIf;
                TheSongSortMgr->OnHighlightChanged();
            }
        }
    }
};

void SongSort::UpdateHighlight() {
    NavListSort::UpdateHighlight();
    TheSongSortMgr->OnHighlightChanged();
};

void SongSort::OnSelectShortcut(int i1) {
    NavListSort::OnSelectShortcut(i1);
    TheSongSortMgr->OnHighlightChanged();
};

void SongSort::Text(int i1, int i2, UIListLabel *listlabel, UILabel *uilabel) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uilabel);
    MILO_ASSERT(app_label, 0x100);
    app_label->SetFromSongSelectNode(unk30[i2]);
};

Symbol SongSort::DetermineHeaderSymbolFromSong(Symbol sym) { return gNullStr; };
