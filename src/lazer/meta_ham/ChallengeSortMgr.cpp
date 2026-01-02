#include "ChallengeSortMgr.h"

#include "ChallengeSortNode.h"

ChallengeRecord::ChallengeRecord(const ChallengeRecord &other) : mChallengeRow(other.mChallengeRow), unk40(other.unk40), unk44(other.unk44), unk48(other.unk48), unk4c(other.unk4c), unk50(other.unk50) {}
ChallengeRecord::~ChallengeRecord() {}
ChallengeRecord &ChallengeRecord::operator=(const ChallengeRecord &other) {
    this->mChallengeRow = other.mChallengeRow;
    this->unk40 = other.unk40;
    this->unk44 = other.unk44;
    this->unk48 = other.unk48;
    this->unk4c = other.unk4c;
    this->unk50 = other.unk50;
    return *this;
}

ChallengeSortMgr::ChallengeSortMgr(SongPreview &preview) : NavListSortMgr(preview) {
    SetName("challenge_provider", ObjectDir::Main());
}

ChallengeSortMgr::~ChallengeSortMgr() {}

void ChallengeSortMgr::Init(SongPreview &preview) {
    MILO_ASSERT(!TheChallengeSortMgr, 0x18);
    TheChallengeSortMgr = new ChallengeSortMgr(preview);
    TheContentMgr.RegisterCallback(TheChallengeSortMgr, false);
}

void ChallengeSortMgr::Terminate() {
    TheContentMgr.UnregisterCallback(TheChallengeSortMgr, false);
    MILO_ASSERT(TheChallengeSortMgr, 0x22);
    //something else here
    TheChallengeSortMgr = nullptr;
}

int ChallengeSortMgr::GetTotalXpEarned(int i1) {
    NavListNode *highlight = GetHighlightItem()->Parent();
    NavListNode *header = dynamic_cast<ChallengeHeaderNode *>(highlight);
    MILO_ASSERT(header, 0xcc);
    return static_cast<ChallengeHeaderNode *>(header)->GetTotalEarnedExp(i1);
}

int ChallengeSortMgr::GetPotentialChallengeExp(int i1) {
    if (IsIndexHeader(i1)) {
        //ChallengeHeaderNode *a = (Sorts()[mCurrentSortIdx]->GetList()[i1]);
        //auto potential = GetPotentialChallengeExp();
    }
    else {
        auto highlight = GetHighlightItem();
        NavListNode *header = dynamic_cast<ChallengeHeaderNode *>(highlight);
        MILO_ASSERT(header, 0xa5);
        return static_cast<ChallengeHeaderNode *>(header)->GetPotentialChallengeExp(highlight);
    }
}

/*String ChallengeSortMgr::GetSongTitle(int i1) {
    if (IsIndexHeader(i1)) {}

}*/

int ChallengeSortMgr::GetOwnerChallengeScore(int songID) {
    for (int i = 0; i < mChallengeRecords.size(); i++) {
        if (songID == mChallengeRecords[i].GetChallengeRow().mSongID && mChallengeRecords[i].GetUnk48() == mChallengeRecords[i].GetUnk4c()) {
            return mChallengeRecords[i].GetChallengeRow().mScore;
        }
    }
    return 0;
}