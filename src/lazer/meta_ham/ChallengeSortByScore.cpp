#include "ChallengeSortByScore.h"
#include "ChallengeRecord.h"
#include "ChallengeSortNode.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/NavListNode.h"
#include "utl/Symbol.h"

int ChallengeScoreCmp::Compare(NavListItemSortCmp const *cmp, NavListNodeType type) const {
    switch (type) {
    case kNodeShortcut:
        return 0;
        break;
    case kNodeHeader:
        break;
    case kNodeItem:
        break;
    default:
        MILO_FAIL("invalid type of node comparison.\n");
        return 0;
        break;
    }

    return -1;
}

NavListShortcutNode *ChallengeSortByScore::NewShortcutNode(NavListItemNode *node) const {
    static Symbol global_challenge("global_challenge");
    static Symbol dlc_challenge("dlc_challenge");
    int type = node->GetCmp()->GetChallengeScoreCmp()->mType;
    Symbol name;
    if (type == 0) {
        name = global_challenge;
    } else if (type == 1) {
        name = dlc_challenge;
    } else {
        name = node->GetToken();
    }
    ChallengeScoreCmp *cmp =
        new ChallengeScoreCmp(type, 0, node->GetCmp()->GetChallengeScoreCmp()->mName);
    return new NavListShortcutNode(cmp, name, true);
}

NavListHeaderNode *ChallengeSortByScore::NewHeaderNode(NavListItemNode *node) const {
    static Symbol global_challenge("global_challenge");
    static Symbol dlc_challenge("dlc_challenge");
    int type = node->GetCmp()->GetChallengeScoreCmp()->mType;
    Symbol name;
    if (type == 0) {
        name = global_challenge;
    } else if (type == 1) {
        name = dlc_challenge;
    } else {
        name = node->GetToken();
    }
    ChallengeScoreCmp *cmp =
        new ChallengeScoreCmp(type, 0, node->GetCmp()->GetChallengeScoreCmp()->mName);
    return new ChallengeHeaderNode(cmp, name, true);
}

NavListItemNode *ChallengeSortByScore::NewItemNode(void *p1) const {
    ChallengeRecord *record = static_cast<ChallengeRecord *>(p1);
    int score = record->GetChallengeRow().mScore;
    Symbol sym = record->GetUnk44();
    int songID = record->GetChallengeRow().mSongID;

    int type = 2;
    if (songID == TheChallenges->GetGlobalChallengeSongID()) {
        type = 0;
    } else {
        songID = record->GetChallengeRow().mSongID;
        if (songID == TheChallenges->GetDlcChallengeSongID()) {
            type = 1;
        }
    }
    ChallengeScoreCmp *cmp = new ChallengeScoreCmp(type, score, sym.Str());
    return new ChallengeSortNode(cmp, record);
}
