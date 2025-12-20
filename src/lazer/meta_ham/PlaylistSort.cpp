#include "meta_ham/PlaylistSort.h"
#include "NavListNode.h"
#include "PlaylistSortMgr.h"
#include "macros.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/NavListSort.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"

PlaylistSort::PlaylistSort() {}

void PlaylistSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    ThePlaylistSortMgr->ClearHeaders();
}

void PlaylistSort::UpdateHighlight() {
    NavListSort::UpdateHighlight();
    ThePlaylistSortMgr->OnHighlightChanged();
}

void PlaylistSort::OnSelectShortcut(int i) {
    NavListSort::OnSelectShortcut(i);
    ThePlaylistSortMgr->OnHighlightChanged();
}

void PlaylistSort::Text(int, int data, UIListLabel *uiListLabel, UILabel *uiLabel) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x96);
    app_label->SetFromPlaylistSelectNode(unk30[data]);
}

void PlaylistSort::SetHighlightedIx(int i) {
    unk54 = unk50;
    if (i >= 0 && GetListSize() >= i) {
        unk50 = mList[i];
        ThePlaylistSortMgr->OnHighlightChanged();
        return;
    }
    unk50 = nullptr;
}
