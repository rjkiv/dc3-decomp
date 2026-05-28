#include "SongSort.h"

#include "AppLabel.h"
#include "ChallengeSort.h"
#include "NavListSort.h"
#include "SongRecord.h"
#include "SongSortMgr.h"
#include "game/GameMode.h"
#include "macros.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/SongRecord.h"
#include "meta_ham/SongSortMgr.h"
#include "meta_ham/SongSortNode.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

SongSort::SongSort() {};

void SongSort::BuildTree() {
    NavListSort::DeleteTree();
    Init();
    std::vector<NavListItemNode *> nodes;

    auto &map = TheSongSortMgr->GetUnk78();
    FOREACH (it, map) {
        SongRecord *record = &it->second;
        NavListItemNode *node = NewItemNode(record);
        auto bound = std::lower_bound(nodes.begin(), nodes.end(), node, CompareHeaders());
        nodes.insert(bound, node);
    }
    bool b = false;
    int count = 0;
    auto begin = nodes.begin();
    auto rangeStart = nodes.begin();

    while (begin != nodes.end()) {
        auto range =
            std::equal_range(nodes.begin(), nodes.end(), *rangeStart, CompareHeaders());
        int rangeSize = range.second - range.first;
        int remaining = nodes.end() - rangeStart;
        count += rangeSize;
        int leftover = remaining - rangeSize;

        static Symbol by_song("by_song");
        static Symbol by_artist("by_artist");
        if (leftover <= 0 || count >= 4
            || (mSortName != by_song
                && (mSortName != by_artist || TheSongSortMgr->IsInHeaderMode()))) {
            auto it = rangeStart;
            if ((b && count <= 12) || !b) {
                it = range.second;
            }

            NavListShortcutNode *shortcut = NewShortcutNode(*begin);
            unk30.push_back(shortcut);
            shortcut->InsertHeaderRange(begin, it, this);

            count = 0;
            b = false;
            begin = it;
            rangeStart = it;
        } else {
            b = true;
            rangeStart = range.second;
        }
    }

    FOREACH (it, unk30) {
        (*it)->FinishSort(this);
    }
}

void SongSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    TheSongSortMgr->ClearHeaders();
};

void SongSort::BuildItemList() {
    Symbol sym = gNullStr;
    NavListSortNode *sortNode = unk50;
    if (sortNode) {
        NavListNodeType type = sortNode->GetType();
        if (type == kNodeFunction) {
            sym = sortNode->GetToken();
        }
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
        SongFunctionNode *randomSongNode = new SongFunctionNode(
            nullptr, random_song, "ui/image/song_select_setlist_keep.png"
        );
        randomSongNode->SetShortcut(unk30.front());
        unk3c.insert(unk3c.begin(), randomSongNode);
        if (inPerform) {
            static Symbol playlists("playlists");
            SongFunctionNode *playlistNode = new SongFunctionNode(
                nullptr, playlists, "ui/image/song_select_setlist_keep.png"
            );
            playlistNode->SetShortcut(unk30.front());
            unk3c.insert(unk3c.begin(), playlistNode);
        }
    } else if (check) {
        static Symbol finish_setlist("finish_setlist");
        SongFunctionNode *finishSetlistNode = new SongFunctionNode(
            nullptr, finish_setlist, "ui/image/song_select_setlist_keep.png"
        );
        finishSetlistNode->SetShortcut(unk30.front());
        unk3c.insert(unk3c.begin(), finishSetlistNode);
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

    if (!sym.Null()) {
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

Symbol SongSort::DetermineHeaderSymbolFromSong(Symbol sym) {
    auto &map = TheSongSortMgr->GetUnk78();
    auto find = map.find(sym);
    if (find != map.end()) {
        NavListItemNode *node = NewItemNode(&find->second);
        FOREACH (it, unk30) {
            NavListShortcutNode *shortcutNode = *it;
            auto &children = shortcutNode->Children();
            MILO_ASSERT(children.size() == 1, 0xea);
            NavListHeaderNode *header =
                dynamic_cast<NavListHeaderNode *>(shortcutNode->FirstChild());
            MILO_ASSERT(header != NULL, 0xec);
            if (header->Compare(node, kNodeHeader) == 0) {
                delete node;
                return header->GetToken();
            }
        }
        delete node;
    }
    return gNullStr;
};
