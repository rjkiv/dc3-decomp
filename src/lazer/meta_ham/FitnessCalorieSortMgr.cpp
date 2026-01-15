#include "meta_ham/FitnessCalorieSortMgr.h"
#include "FitnessCalorieSortMgr.h"
#include "NavListSort.h"
#include "macros.h"
#include "meta/SongPreview.h"
#include "meta_ham/FitnessCalorieSortNode.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/NavListSortMgr.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "ui/UIPanel.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

FitnessCalorieSortMgr *TheFitnessCalorieSortMgr;

FitnessCalorieSortMgr::FitnessCalorieSortMgr(SongPreview &sp) : NavListSortMgr(sp) {
    SetName("calorie_provider", ObjectDir::Main());
    // mSorts.push_back(new FitnessCalorieSortByCalorie());
    mGroupSize = 1;
}

FitnessCalorieSortMgr::~FitnessCalorieSortMgr() {}

void FitnessCalorieSortMgr::Init(SongPreview &sp) {
    MILO_ASSERT(!TheFitnessCalorieSortMgr, 0x17);
    TheFitnessCalorieSortMgr = new FitnessCalorieSortMgr(sp);
    TheContentMgr.RegisterCallback(TheFitnessCalorieSortMgr, false);
}

void FitnessCalorieSortMgr::Terminate() {
    TheContentMgr.UnregisterCallback(TheFitnessCalorieSortMgr, false);
    MILO_ASSERT(TheFitnessCalorieSortMgr, 0x21);
    if (!TheFitnessCalorieSortMgr) {
        TheFitnessCalorieSortMgr = nullptr;
        return;
    }
    RELEASE(TheFitnessCalorieSortMgr);
}

void FitnessCalorieSortMgr::OnEnter() {
    FOREACH (it, mSorts) {
        (*it)->BuildTree();
    }
    NavListSort *sort = mSorts[mCurrentSortIdx];
    sort->BuildItemList();
    if (unk48) {
        sort->SetHighlightID(unk44);
        unk48 = false;
    }
    sort->UpdateHighlight();
}

Symbol FitnessCalorieSortMgr::MoveOn() {
    FitnessCalorieSortNode *node =
        dynamic_cast<FitnessCalorieSortNode *>(NavListSortMgr::GetHighlightItem());
    MILO_ASSERT(node, 0x55);
    UIPanel *fitnessPanel = ObjectDir::Main()->Find<UIPanel>("fitness_panel");
    if (fitnessPanel && fitnessPanel->GetState() == 1) {
        fitnessPanel->Handle(Message("calorie_selected", node->GetUnk48()), true);
    }
    return gNullStr;
}
