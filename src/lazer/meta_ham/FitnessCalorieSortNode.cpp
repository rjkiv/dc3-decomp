#include "meta_ham/FitnessCalorieSortNode.h"
#include "FitnessCalorieSortMgr.h"
#include "NavListNode.h"
#include "meta_ham/AppLabel.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

#pragma region FitnessCalorieSortNode

FitnessCalorieSortNode::FitnessCalorieSortNode(NavListItemSortCmp *cmp, int i)
    : NavListSortNode(cmp) {}

Symbol FitnessCalorieSortNode::GetToken() const {
    return MakeString("calorie_node_%i", unk48);
}

Symbol FitnessCalorieSortNode::OnSelect() { return TheFitnessCalorieSortMgr->MoveOn(); }

void FitnessCalorieSortNode::Text(UIListLabel *uiListLabel, UILabel *uiLabel) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0xee);
    if (uiListLabel->Matches("calorie")) {
        String s = MakeString(
            "%i %s", unk48, Localize("fitness_goal_calories_generic", 0, TheLocale)
        );
        uiLabel->SetPrelocalizedString(s);
    } else {
        uiLabel->SetTextToken(gNullStr);
    }
}

#pragma endregion FitnessCalorieSortNode
#pragma region FitnessCalorieHeaderNode

FitnessCalorieHeaderNode::FitnessCalorieHeaderNode(
    NavListItemSortCmp *cmp, Symbol s, bool b
)
    : NavListHeaderNode(cmp, s, b), unk58() {}

void FitnessCalorieHeaderNode::OnHighlight() { SetCollapseStateIcon(true); }

Symbol FitnessCalorieHeaderNode::OnSelect() {
    if (!TheFitnessCalorieSortMgr->IsInHeaderMode()) {
        TheFitnessCalorieSortMgr->SetHeaderMode(true);
        TheFitnessCalorieSortMgr->SetEnteringHeaderMode(true);
    } else {
        TheFitnessCalorieSortMgr->SetEnteringHeaderMode(false);
        TheFitnessCalorieSortMgr->SetExitingHeaderMode(true);
    }
    return gNullStr;
}

Symbol FitnessCalorieHeaderNode::OnSelectDone() {
    if (TheFitnessCalorieSortMgr->EnteringHeaderMode()) {
        TheFitnessCalorieSortMgr->SetEnteringHeaderMode(false);
    }
    if (TheFitnessCalorieSortMgr->ExitingHeaderMode()) {
        if (TheFitnessCalorieSortMgr->IsInHeaderMode()) {
            TheFitnessCalorieSortMgr->SetHeaderMode(false);
        }
        TheFitnessCalorieSortMgr->SetExitingHeaderMode(false);
    }
    TheFitnessCalorieSortMgr->OnEnter();
    TheFitnessCalorieSortMgr->GetCurrentSort()->BuildItemList();
    return gNullStr;
}

void FitnessCalorieHeaderNode::SetCollapseStateIcon(bool b) const {
    Symbol s = gNullStr;
    UILabel *iconLabel = GetCollapseIconLabel();
    if (iconLabel) { // idk what the vfunc is
        static Symbol header_open_icon("header_open_icon");
        static Symbol header_open_highlighted_icon("header_open_highlighted_icon");
        static Symbol header_closed_icon("header_closed_icon");
        static Symbol header_closed_highlighted_icon("header_closed_highlighted_icon");
        if (TheFitnessCalorieSortMgr->IsInHeaderMode()) {
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

void FitnessCalorieHeaderNode::Text(UIListLabel *uiListLabel, UILabel *uiLabel) const {
    AppLabel *app_label = dynamic_cast<AppLabel *>(uiLabel);
    MILO_ASSERT(app_label, 0x94);
    if (uiListLabel->Matches("sort_header")) {
    }
}

#pragma endregion FitnessCalorieHeaderNode
