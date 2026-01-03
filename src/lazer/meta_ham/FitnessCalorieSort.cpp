#include "meta_ham/FitnessCalorieSort.h"
#include "FitnessCalorieSortMgr.h"
#include "NavListSort.h"
#include "meta_ham/AppLabel.h"
#include "os/Debug.h"
#include "rndobj/Mesh.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"

FitnessCalorieSort::FitnessCalorieSort() {}

void FitnessCalorieSort::Text(
    int, int idx, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0xa4);
    app_label->SetFromGeneralSelectNode(unk30[idx]);
}
void FitnessCalorieSort::DeleteItemList() {
    NavListSort::DeleteItemList();
    TheFitnessCalorieSortMgr->ClearHeaders();
}

void FitnessCalorieSort::SetHighlightedIx(int idx) {
    unk54 = unk50;
    if (0 <= idx) {
        if (mList.size() >= idx) {
            if (mList.size() == 0) {
                return;
            }
            unk50 = mList[idx];
            TheFitnessCalorieSortMgr->OnHighlightChanged();
            return;
        }
    }
    unk50 = 0;
}

void FitnessCalorieSort::UpdateHighlight() {
    if (mList.size() != 0) {
        NavListSort::UpdateHighlight();
        TheFitnessCalorieSortMgr->OnHighlightChanged();
    }
}

void FitnessCalorieSort::OnSelectShortcut(int i) {
    if (mList.size() != 0) {
        NavListSort::OnSelectShortcut(i);
        TheFitnessCalorieSortMgr->OnHighlightChanged();
    }
}
