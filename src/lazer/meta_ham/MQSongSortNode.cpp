#include "MQSongSortNode.h"

#include "MQSongSortMgr.h"

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
    }
    TheMQSongSortMgr->SetEnteringHeaderMode(TheMQSongSortMgr->IsInHeaderMode() == 0);
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
    return TheMQSongSortMgr->HeadersSelectable() != 0; // fruity function not sure whats going on
}

const char *MQSongHeaderNode::GetAlbumArtPath() {
    static Symbol by_album("by_album");
    static Symbol singles("singles");

    NavListSort *sort = TheMQSongSortMgr->GetCurrentSort();

    if (sort->GetSortName() != by_album) {
        return "";
    }
    if (unk5c == singles) {
        return "";
    }
    return TheMQSongSortMgr->GetHighlightItem()->GetAlbumArtPath();
}

void MQSongHeaderNode::Text(UIListLabel *listlabel, UILabel *label) const {
    if (listlabel->Matches("song")) {

    }
}

NavListSortNode *MQSongHeaderNode::GetFirstActive() {
    FOREACH(it, Children()) {
        NavListSortNode *node = (*it)->GetFirstActive();
        if (node) {
            return node;
        }
    }
    if (!TheMQSongSortMgr->HeadersSelectable()) {
        return this;
    }
}

