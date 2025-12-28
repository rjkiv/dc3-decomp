#include "MQSongSortMgr.h"

//MQSongSortMgr::MQSongSortMgr(SongPreview &) {};

MQSongSortMgr::~MQSongSortMgr() {};

MQSongSort::MQSongSort() {};

void MQSongSortMgr::Init(SongPreview &preview) {
    MILO_ASSERT(!TheMQSongSortMgr, 0x18);
    TheMQSongSortMgr = new MQSongSortMgr(preview);
    TheContentMgr.RegisterCallback(TheMQSongSortMgr, false);
}

void MQSongSortMgr::OnEnter() {
    mHeadersSelectable = true;
    UpdateList();

    //for (int i = 0; i < mSorts.size(); i++) {
    //    mSorts[i]->NumData() + mHeadersSelectable;
    //}
    if (unk48) {
        mSorts[mCurrentSortIdx]->SetHighlightID(unk44);
        unk48 = false;
    }
    mSorts[mCurrentSortIdx]->BuildTree();
}

Symbol MQSongSortMgr::MoveOn() {
    MILO_ASSERT(0, 0x45);
    return gNullStr;
}

bool MQSongSortMgr::SelectionIs(Symbol sym) {
    static Symbol challenge("challenge");
    static Symbol header("header");
    NavListSortNode *sortNode;
    if (sym == challenge) {
        sortNode = NavListSortMgr::GetHighlightItem();
    }
    else {
        if (sym != header) {
            return false;
        }
        sortNode = NavListSortMgr::GetHighlightItem();
    }

    return sortNode->GetFirstActive() == 0;
}

bool MQSongSortMgr::IsCharacter(Symbol sym) const {
    FOREACH(it, unk78) {
        if (*it == sym) {
            return true;
        }
    }
    return false;
}