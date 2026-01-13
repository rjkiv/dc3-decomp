#include "SongSort.h"

#include "AppLabel.h"
#include "ChallengeSort.h"
#include "SongSortMgr.h"
#include "game/GameMode.h"
#include "meta_ham/NavListNode.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"

SongSort::SongSort() {};

void SongSort::BuildTree() {};

void SongSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    TheSongSortMgr->ClearHeaders();
};

void SongSort::BuildItemList() {
    Symbol sym(gNullStr);
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
    auto prop = TheGameMode->Property(song_select_mode, true)->Sym();

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
    AppLabel *app_label = dynamic_cast<AppLabel*>(uilabel);
    MILO_ASSERT(app_label, 0x100);
    app_label->SetFromSongSelectNode(unk30[i2]);
};

Symbol SongSort::DetermineHeaderSymbolFromSong(Symbol sym) { return sym; };
