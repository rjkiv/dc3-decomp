#include "ChallengeSortByScore.h"
#include "ChallengeRecord.h"
#include "ChallengeSortNode.h"
#include "meta/Sorting.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/NavListNode.h"
#include "utl/Symbol.h"
#include <cstring>

int ChallengeScoreCmp::Compare(NavListItemSortCmp const *cmp, NavListNodeType type) const {
    const ChallengeScoreCmp *mCmp;
    switch (type) {
    case kNodeShortcut:
        break;
    case kNodeHeader:
        mCmp = cmp->GetChallengeScoreCmp();
        if (mChallengeSongType == mCmp->mChallengeSongType) {
            return AlphaKeyStrCmp(mSongName, mCmp->mSongName, false);
        }
        if (mChallengeSongType < mCmp->mChallengeSongType) {
            return -1;
        }
        return 1;
        break;
    case kNodeItem:
        mCmp = cmp->GetChallengeScoreCmp();
        if (mScore != mCmp->mScore) {
            if (mScore <= mCmp->mScore) {
                return 1;
            }
            return -1;
        }
        break;
    default:
        MILO_FAIL("invalid type of node comparison.\n");
        return 0;
        break;
    }

    return 0;
}

NavListShortcutNode *ChallengeSortByScore::NewShortcutNode(NavListItemNode *node) const {
    static Symbol global_challenge("global_challenge");
    static Symbol dlc_challenge("dlc_challenge");
    int type = node->GetCmp()->GetChallengeScoreCmp()->mChallengeSongType;
    Symbol name;
    if (type == 0) {
        name = global_challenge;
    } else if (type == 1) {
        name = dlc_challenge;
    } else {
        name = node->GetToken();
    }
    ChallengeScoreCmp *cmp =
        new ChallengeScoreCmp(type, 0, node->GetCmp()->GetChallengeScoreCmp()->mSongName);
    return new NavListShortcutNode(cmp, name, true);
}

NavListHeaderNode *ChallengeSortByScore::NewHeaderNode(NavListItemNode *node) const {
    static Symbol global_challenge("global_challenge");
    static Symbol dlc_challenge("dlc_challenge");
    int type = node->GetCmp()->GetChallengeScoreCmp()->mChallengeSongType;
    Symbol name;
    if (type == 0) {
        name = global_challenge;
    } else if (type == 1) {
        name = dlc_challenge;
    } else {
        name =
            static_cast<ChallengeSortNode *>(node)->GetChallengeRecord()->GetSongTitle();
    }
    ChallengeScoreCmp *cmp =
        new ChallengeScoreCmp(type, 0, node->GetCmp()->GetChallengeScoreCmp()->mSongName);
    return new ChallengeHeaderNode(cmp, name, true);
}

NavListItemNode *ChallengeSortByScore::NewItemNode(void *p1) const {
    ChallengeRecord *record = static_cast<ChallengeRecord *>(p1);
    int score = record->GetChallengeRow().score;
    const char *songTitle = record->GetSongTitle();
    int songID = record->GetChallengeRow().songId;

    int type = 2;
    if (songID == TheChallenges->GetGlobalChallengeSongID()) {
        type = 0;
    } else {
        songID = record->GetChallengeRow().songId;
        if (songID == TheChallenges->GetDlcChallengeSongID()) {
            type = 1;
        }
    }
    ChallengeScoreCmp *cmp = new ChallengeScoreCmp(type, score, songTitle);
    return new ChallengeSortNode(cmp, record);
}
