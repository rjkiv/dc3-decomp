#include "ChallengeSortByScore.h"

#include "ChallengeSortNode.h"

ChallengeSortNode::ChallengeSortNode(NavListItemSortCmp *cmp, ChallengeRecord *record) : NavListItemNode(cmp), mChallengeRecord(record){  }