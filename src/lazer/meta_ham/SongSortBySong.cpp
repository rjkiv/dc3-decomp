#include "SongSortBySong.h"

#include "meta/Sorting.h"

int SongCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    switch (type) {
    case kNodeShortcut:
        return 0;

    case kNodeHeader: {
        const SongCmp *songCmp = cmp->GetSongCmp();
        int iVar3 = unk4 - songCmp->unk4;
        if (unk8 == 0) return iVar3;
        if (0 < iVar3) return iVar3;
        return unk8 - songCmp->unk4 >> 31 & unk8 - songCmp->unk4; //something strange here
    }
    case kNodeItem: {
        const SongCmp *songCmp = cmp->GetSongCmp();
        return AlphaKeyStrCmp(unk4, songCmp->unk4, false);
    }
    default:
        MILO_FAIL("invalid type of node comparison.\n");
    }
    return 0;
}