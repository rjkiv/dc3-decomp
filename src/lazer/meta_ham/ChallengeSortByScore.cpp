#include "ChallengeSortByScore.h"

#include "ChallengeSortNode.h"

ChallengeSortNode::ChallengeSortNode(NavListItemSortCmp *cmp, ChallengeRecord *record) : NavListItemNode(cmp), unk48(record){  }