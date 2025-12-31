#include "SongSortByDiff.h"

#include "HamSongMgr.h"
#include "meta/SongMgr.h"
#include "SongRecord.h"
#include "meta/Sorting.h"

int DifficultyCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    switch (type) {
        case kNodeShortcut:
            return 0;

        case kNodeHeader: {
            const DifficultyCmp *diffCmp = cmp->GetDifficultyCmp();
            if (mTier == diffCmp->mTier) return 0;
            if (diffCmp->mTier == -1)   return -1;
            if (mTier == -1)    return 1;
            if (mTier < diffCmp->mTier) return -1;
            else return 1;
        }
        case kNodeItem: {
            const DifficultyCmp *diffCmp = cmp->GetDifficultyCmp();
            float other = diffCmp->mRank;
            float mine = mRank;
            if (mine == other) return AlphaKeyStrCmp(mName, diffCmp->mName, false);
            if (other == 0) return -1;
            if (mine == 0) return 1;
            if (mine < other) return -1;
            else return 1;
        }
        default:
            MILO_FAIL("invalid type of node comparison.\n");
    }
    return 0;
}

SongSortByDiff::SongSortByDiff() {
    static Symbol by_difficulty = "by_difficulty";
}

NavListShortcutNode *SongSortByDiff::NewShortcutNode(NavListItemNode *node) const {
    DifficultyCmp *diffcmp = (DifficultyCmp *)node;
    int tier = diffcmp->mTier;
    static Symbol no_part("no_part");
    DifficultyCmp *cmp = new DifficultyCmp(tier, 0, "");
    Symbol sym = tier == -1 ? no_part : TheHamSongMgr.RankTierToken(tier);
    NavListShortcutNode *newNode = new NavListShortcutNode(cmp, sym, true);
    return newNode;
}
