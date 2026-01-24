#include "MQSongSortNode.h"

#include "AppLabel.h"
#include "HamUI.h"
#include "MQSongSortMgr.h"
#include "HamStarsDisplay.h"
#include "meta_ham/NavListNode.h"
#include "stl/_vector.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

BEGIN_HANDLERS(MQSongHeaderNode)
    HANDLE_EXPR(get_challenge_count, unk58)
    HANDLE_SUPERCLASS(NavListHeaderNode)
END_HANDLERS

MQSongHeaderNode::MQSongHeaderNode(NavListItemSortCmp *cmp, Symbol sym, bool b)
    : NavListHeaderNode(cmp, sym, b), unk5c(0), unk58(0) {}

void MQSongHeaderNode::OnHighlight() {
    unk5c = true;
    SetCollapseStateIcon(true);
}

void MQSongHeaderNode::OnUnHighlight() {
    unk5c = false;
    SetCollapseStateIcon(false);
}

Symbol MQSongHeaderNode::OnSelect() {
    if (!TheMQSongSortMgr->IsInHeaderMode()) {
        TheMQSongSortMgr->SetHeaderMode(true);
        TheMQSongSortMgr->SetEnteringHeaderMode(true);
    } else
        TheMQSongSortMgr->SetEnteringHeaderMode(false);
    return gNullStr;
}

Symbol MQSongHeaderNode::OnSelectDone() {
    if (TheMQSongSortMgr->IsInHeaderMode() && !TheMQSongSortMgr->EnteringHeaderMode()) {
        TheMQSongSortMgr->SetHeaderMode(false);
    }
    TheMQSongSortMgr->OnEnter();
    NavListSort *sort = TheMQSongSortMgr->GetCurrentSort();
    sort->BuildItemList();
    return gNullStr;
}

bool MQSongHeaderNode::IsActive() const {
    return TheMQSongSortMgr->HeadersSelectable() != 0; // fruity function not sure whats
                                                       // going on
}

const char *MQSongHeaderNode::GetAlbumArtPath() {
    static Symbol by_album("by_album");
    static Symbol singles("singles");

    NavListSort *sort = TheMQSongSortMgr->GetCurrentSort();

    if (sort->GetSortName() == by_album && GetToken() != singles && !mChildren.empty())
        return mChildren.front()->GetAlbumArtPath();
    else
        return 0;
}

void MQSongHeaderNode::Text(UIListLabel *listlabel, UILabel *label) const {
    if (listlabel->Matches("song")) {
        // const char *c = mCmp->GetMQSongCharCmp()->unk8;
        // label->SetTextToken(MakeString("mqheader_%s", c));
    } else {
        if (!listlabel->Matches("song_prefix")) {
            if (!listlabel->Matches("header_collapse")) {
                return;
            }
            SetCollapseStateIcon(unk5c);
            return;
        }
        label->SetTextToken(gNullStr);
    }
}

void MQSongHeaderNode::SetCollapseStateIcon(bool b) const {
    Symbol s = gNullStr;
    UILabel *iconLabel = GetCollapseIconLabel();
    if (iconLabel) {
        static Symbol header_open_icon("header_open_icon");
        static Symbol header_open_highlighted_icon("header_open_highlighted_icon");
        static Symbol header_closed_icon("header_closed_icon");
        static Symbol header_closed_highlighted_icon("header_closed_highlighted_icon");
        if (TheMQSongSortMgr->IsInHeaderMode()) {
            if (b) {
                s = header_closed_highlighted_icon;
            } else {
                s = header_closed_icon;
            }
        } else {
            if (b) {
                s = header_open_highlighted_icon;
            } else {
                s = header_open_icon;
            }
        }
        iconLabel->SetTextToken(s);
    }
}

NavListSortNode *MQSongHeaderNode::GetFirstActive() {
    FOREACH (it, Children()) {
        NavListSortNode *node = (*it)->GetFirstActive();
        if (node) {
            return node;
        }
    }
    if (!TheMQSongSortMgr->HeadersSelectable()) {
        return this;
    }
}

void MQSongHeaderNode::Renumber(std::vector<NavListSortNode *> &vec) {
    mStartIx = vec.size();
    if (TheMQSongSortMgr->GetHeadersSelectable()) {
        vec.push_back(this);
        TheMQSongSortMgr->AddHeaderIndex(mStartIx);
    }
    if (!TheMQSongSortMgr->IsInHeaderMode()) {
        FOREACH (it, mChildren) {
            (*it)->Renumber(vec);
        }
    }
}

Symbol MQSongSortNode::OnSelect() { return Select(); }

void MQSongSortNode::Text(UIListLabel *listlabel, UILabel *label) const {
    if (listlabel->Matches("song")) {
        AppLabel *pAppLabel = dynamic_cast<AppLabel *>(label);
        MILO_ASSERT(pAppLabel, 0x10f);
        pAppLabel->SetBlacklightSongName(unk48, -1, false);
    } else if (listlabel->Matches("song_prefix")) {
        AppLabel *pAppLabel = dynamic_cast<AppLabel *>(label);
        MILO_ASSERT(pAppLabel, 0x116);
        if (IsHeader() || !TheHamUI.IsBlacklightMode()) {
            label->SetTextToken(gNullStr);
        } else {
            static Symbol song_select_song_prefix("song_select_song_prefix");
            label->SetTextToken(song_select_song_prefix);
        }
    } else {
        label->SetTextToken(listlabel->GetDefaultText());
    }
}

void MQSongSortNode::Custom(UIListCustom *list, Hmx::Object *obj) const {
    if (list->Matches("stars")) {
        HamStarsDisplay *pStarDisplay = dynamic_cast<HamStarsDisplay *>(obj);
        MILO_ASSERT(pStarDisplay, 0x12d);
        static DataNode &mq_difficulty = DataVariable("mq_difficulty");
        Difficulty difficulty = (Difficulty)mq_difficulty.Int();
        pStarDisplay->SetShowing(true);
        int songID = TheSongMgr.GetSongIDFromShortName(unk48, true);
        pStarDisplay->SetSongWithDifficulty(songID, difficulty, false);
    }
}
