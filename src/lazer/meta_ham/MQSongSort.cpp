#include "MQSongSort.h"

#include "AppLabel.h"
#include "MQSongSortMgr.h"

MQSongSort::MQSongSort() {};

void MQSongSort::DeleteItemList() {
    TheMQSongSortMgr->ClearHeaders();
}

void MQSongSort::SetHighlightedIx(int i1) {
    unk54 = unk50;
    if (i1 >= 0) {
        if (mList.size() >= i1) {
            if (mList.size() == 0) {
                return;
            }
            unk50 = mList[i1];
            TheMQSongSortMgr->OnHighlightChanged();
            return;
        }
    }
    unk50 = 0;
}

void MQSongSort::UpdateHighlight() {
    if (mList.size() != 0) {
        NavListSort::UpdateHighlight();
        TheMQSongSortMgr->OnHighlightChanged();
    }
}

void MQSongSort::OnSelectShortcut(int i1) {
    if (mList.size() != 0) {
        NavListSort::OnSelectShortcut(i1);
        TheMQSongSortMgr->OnHighlightChanged();
    }
}

void MQSongSort::Text(int i1, int i2, UIListLabel *listlabel, UILabel *label) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0xd0);

    app_label->SetFromGeneralSelectNode(unk30[i2]);
}

void MQSongSort::SetHighlightItem(const NavListSortNode *node) {
    NavListSortNode *node2 = unk50;
    unk50 = 0;
    unk54 = node2;
    if (node) {
        Symbol sortName = node->GetToken();
        if (sortName == 5 || sortName == 4) {
            // TODO: needs SortNodeFind class - maybe part of NavListNode?
        }
    }
}