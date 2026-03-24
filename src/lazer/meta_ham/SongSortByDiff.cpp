#include "SongSortByDiff.h"

#include "HamSongMetadata.h"
#include "HamSongMgr.h"
#include "SongSortNode.h"
#include "meta/SongMgr.h"
#include "SongRecord.h"
#include "meta/Sorting.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/NavListNode.h"

int DifficultyCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    switch (type) {
    case kNodeShortcut:
        return 0;

    case kNodeHeader: {
        const DifficultyCmp *diffCmp = cmp->GetDifficultyCmp();
        if (mTier == diffCmp->mTier)
            return 0;
        if (diffCmp->mTier == -1)
            return -1;
        if (mTier == -1)
            return 1;
        if (mTier < diffCmp->mTier)
            return -1;
        else
            return 1;
    }
    case kNodeItem: {
        const DifficultyCmp *diffCmp = cmp->GetDifficultyCmp();
        float other = diffCmp->mRank;
        float mine = mRank;
        if (mine == other)
            return AlphaKeyStrCmp(mName, diffCmp->mName, false);
        if (other == 0)
            return -1;
        if (mine == 0)
            return 1;
        if (mine < other)
            return -1;
        else
            return 1;
    }
    default:
        MILO_FAIL("invalid type of node comparison.\n");
    }
    return 0;
}

NavListShortcutNode *SongSortByDiff::NewShortcutNode(NavListItemNode *node) const {
    int tier = node->GetCmp()->GetDifficultyCmp()->mTier;
    DifficultyCmp *cmp = new DifficultyCmp(tier, 0, "");
    static Symbol no_part("no_part");
    Symbol s = (tier != -1) ? TheHamSongMgr.RankTierToken(tier) : no_part;
    return new NavListShortcutNode(cmp, s, true);
}

NavListHeaderNode *SongSortByDiff::NewHeaderNode(NavListItemNode *node) const {
    int tier = node->GetCmp()->GetDifficultyCmp()->mTier;
    DifficultyCmp *newCmp = new DifficultyCmp(tier, 0, "");
    static Symbol no_part("no_part");
    Symbol tierToken = tier != -1 ? TheHamSongMgr.RankTierToken(tier) : no_part;
    return new SongHeaderNode(newCmp, tierToken, true);
}

NavListItemNode *SongSortByDiff::NewItemNode(void *p1) const {
    SongRecord *record = static_cast<SongRecord *>(p1);
    int tier = record->GetTier();
    float rank = record->Metadata()->Rank();
    const char *title = record->Metadata()->Title();
    DifficultyCmp *cmp = new DifficultyCmp(tier, rank, title);
    return new SongSortNode(cmp, record);
}
