#include "MQSongSortByCharacter.h"

int MQSongCharCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    switch (type) {
    case kNodeShortcut:
        return 0;

    case kNodeHeader: {
        const MQSongCharCmp *mqCmp = cmp->GetMQSongCharCmp();
        return strcmp(unk8, mqCmp->unk8);
    }

    case kNodeItem: {
        const MQSongCharCmp *mqCmp = cmp->GetMQSongCharCmp();
        return strcmp(unk4, mqCmp->unk4);
    }
    default:
        MILO_FAIL("invalid type of node comparison.\n");
    }
    return 0;
}