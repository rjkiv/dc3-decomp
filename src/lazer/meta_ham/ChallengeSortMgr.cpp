#include "ChallengeSortMgr.h"

#include "ChallengeSortByScore.h"
#include "ChallengeSortNode.h"
#include "lazer/game/GameMode.h"

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

BEGIN_CUSTOM_HANDLERS(ChallengeSortMgr)
HANDLE_EXPR(get_target_challenge_score, _msg->Int(2)) // li r11, 0x3e8 inst here?
HANDLE_EXPR(get_total_earned_xp, GetTotalXpEarned(_msg->Int(2)))
HANDLE_EXPR(get_challenger_name, GetChallengerName())
HANDLE_EXPR(get_song_id, GetSongID(_msg->Int(2)))
HANDLE_EXPR(get_song_shortname, GetSongShortName(_msg->Int(2)))
HANDLE_EXPR(get_song_title, GetSongTitle(_msg->Int(2)))
HANDLE_EXPR(get_challenger_xp, GetChallengerXp(_msg->Int(2)))
HANDLE_EXPR(get_challenge_record_song_type, GetChallengeRecordSongType(_msg->Int(2)))
HANDLE_SUPERCLASS(NavListSortMgr)
END_CUSTOM_HANDLERS

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
        return static_cast<ChallengeHeaderNode *>(mSorts[mCurrentSortIdx]->GetList()[i1])->GetChallengeExp();
    }
    else {
        auto highlight = GetHighlightItem();
        NavListNode *header = dynamic_cast<ChallengeHeaderNode *>(highlight);
        MILO_ASSERT(header, 0xa5);
        return static_cast<ChallengeHeaderNode *>(header)->GetPotentialChallengeExp(highlight);
    }
}

int ChallengeSortMgr::GetOwnerChallengeScore(int songID) {
    for (int i = 0; i < mChallengeRecords.size(); i++) {
        if (songID == mChallengeRecords[i].GetChallengeRow().mSongID && mChallengeRecords[i].GetUnk48() == mChallengeRecords[i].GetUnk4c()) {
            return mChallengeRecords[i].GetChallengeRow().mScore;
        }
    }
    return 0;
}

int ChallengeSortMgr::GetChallengeExp(int i1) {
    if (IsIndexHeader(i1)) {
        return static_cast<ChallengeHeaderNode *>(mSorts[mCurrentSortIdx]->GetList()[i1])->GetChallengeExp();
    }
    else return static_cast<ChallengeSortNode *>(mSorts[mCurrentSortIdx]->GetList()[i1])->GetChallengeExp();
}

int ChallengeSortMgr::GetSongID(int i1) {
    if (IsIndexHeader(i1)) {
        return static_cast<ChallengeHeaderNode *>(mSorts[mCurrentSortIdx]->GetList()[i1])->GetSongID();
    }
    else return static_cast<ChallengeSortNode *>(mSorts[mCurrentSortIdx]->GetList()[i1])->GetSongID();
}

Symbol ChallengeSortMgr::GetSongShortName(int songID) {
    if (IsIndexHeader(songID)) {
        return static_cast<ChallengeHeaderNode *>(mSorts[mCurrentSortIdx]->GetList()[songID])->GetSongShortName();
    }
    else {
        return mSorts[mCurrentSortIdx]->GetList()[songID]->GetToken();
    }
}

int ChallengeSortMgr::GetOwnerChallengeTimeStamp(int i1) {
    for (int i = 0; i < mChallengeRecords.size(); i++) {
        if (i1 == mChallengeRecords[i].GetChallengeRow().mSongID && mChallengeRecords[i].GetUnk48() == mChallengeRecords[i].GetUnk4c()) {
            return mChallengeRecords[i].GetChallengeRow().mTimeStamp;
        }
    }
    return 0;
}

int ChallengeSortMgr::GetChallengeScore(int i1) {
    if (IsIndexHeader(i1)) {
        return GetBestChallengeScore(GetSongID(i1));
    }
    else {
        return static_cast<ChallengeSortNode *>(mSorts[mCurrentSortIdx]->GetList()[i1])->GetChallengeScore();
    }
}

Symbol ChallengeSortMgr::GetChallengerName() {
    auto node = dynamic_cast<ChallengeSortNode *>(GetHighlightItem());
    MILO_ASSERT(node, 0xdb);
    return node->GetChallengeRecord()->GetUnk4c();
}

int ChallengeSortMgr::GetBestChallengeScore(int songID) {
    int currentHighest = 0;
    for (int i = 0; i < mChallengeRecords.size(); i++) {
        int score = mChallengeRecords[i].GetChallengeRow().mScore;
        if (songID == mChallengeRecords[i].GetChallengeRow().mSongID && currentHighest < score) {
            currentHighest = score;
        }
    }
    return currentHighest;
}

String ChallengeSortMgr::GetSongTitle(int songID) {
    if (IsIndexHeader(songID)) {
        return static_cast<ChallengeHeaderNode *>(mSorts[mCurrentSortIdx]->GetList()[songID])->GetSongShortTitle();;
    }
    else {
        return static_cast<ChallengeSortNode *>(mSorts[mCurrentSortIdx]->GetList()[songID])->GetChallengeRecord()->GetUnk44(); // FIXME
    }
}

int ChallengeSortMgr::GetChallengeRecordSongType(int i1) {
    if (IsIndexHeader(i1)) {
        return -1;
    }
    else {
        return static_cast<ChallengeSortNode *>(mSorts[mCurrentSortIdx]->GetList()[i1])->GetChallengeRecord()->GetChallengeRow().unk38; // needs to grab something at 0x50 - possible wrong cast?
    }
}

Symbol ChallengeSortMgr::MoveOn() {
    static Symbol song_select_quickplay("song_select_quickplay");
    Symbol song_select_mode = TheGameMode->Property("song_select_mode", true)->Sym();
    if (song_select_quickplay == song_select_mode) {
        static Symbol move_on_quickplay("move_on_quickplay");
        UIScreen *panel = ObjectDir::Main()->Find<UIScreen>("challenge_feed_panel"); // FIXME: why uipanel no work
        Symbol moq("move_on_quickplay");
        static Message msg(moq);
        HandleType(msg);
    }
    return 0;
}