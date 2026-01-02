#include "ChallengeSortNode.h"

#include "Challenges.h"
#pragma region ChallengeHeaderNode
ChallengeHeaderNode::ChallengeHeaderNode(NavListItemSortCmp *cmp, Symbol sym, bool b)
    : NavListHeaderNode(cmp, sym, b), unk58(0) {}

int ChallengeHeaderNode::GetChallengeExp() {
    FOREACH(it, Children()) {
        auto node = *it;
        MILO_ASSERT(node, 0xd0);
        //TheChallenges->CalculateChallengeXp((*it)->)
    }
    return 0;
}
#pragma endregion

#pragma region ChallengeSortNode

int ChallengeSortNode::GetChallengeExp() {
    //int challengeXp = TheChallenges->CalculateChallengeXp(unk48->0x24, unk48->0x28);
    return 0;
}

#pragma endregion