#include "SongSortByDiff.h"

#include "HamSongMgr.h"
#include "meta/SongMgr.h"
#include "SongRecord.h"
#include "meta/Sorting.h"

int DifficultyCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    DifficultyCmp *diffcmp = (DifficultyCmp *)cmp;
    switch (type) {
        case NavListNodeType::kNodeShortcut:
            return 0;

        case NavListNodeType::kNodeHeader: {
            auto i = diffcmp->GetDifficultyCmp()->mTier;
            if (i == mTier) {
                return 0;
            }
            if (i == -1) {
                return -1;
            }
            if (mTier == -1) {
                return 1;
            }
            return mTier < diffcmp->mTier ? -1 : 1;
        }
        case NavListNodeType::kNodeItem: {
            float other = diffcmp->mRank;
            float mine = mRank;
            if (mine == other) {
                return AlphaKeyStrCmp(mName, diffcmp->mName, true);
            } else if (other == 0)
                return -1;
            else if (mine == 0)
                return 1;
            else if (mine < other)
                return -1;
            else
                return 1;
        }

        default: {
            MILO_FAIL("invalid type of node comparison.\n");
            return 0;
        }
    }
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
