#include "MQSongSortByCharacter.h"

int MQSongCharCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    switch (type) {
    case kNodeShortcut:
        return 0;

    case kNodeHeader: {
        const MQSongCharCmp *mqCmp = cmp->GetMQSongCharCmp();
        int iVar2 = unk8 - mqCmp->unk8;
        const char *a = unk8;
        const char *b = mqCmp->unk8;
        for (int diff = iVar2; diff == 0; diff = (++a) - (++b)) {
            if (a[diff] == '\0') {
                return diff;
            }
        }
        return a - b;
    }

    case kNodeItem: {
        const MQSongCharCmp *mqCmp = cmp->GetMQSongCharCmp();
        int iVar2 = unk4 - mqCmp->unk4;
        const char *a = unk4;
        const char *b = mqCmp->unk4;
        for (int diff = iVar2; diff == 0; diff = (++a) - (++b)) {
            if (a[diff] == '\0') {
                return diff;
            }
        }
        return a - b;
    }
    default:
        MILO_FAIL("invalid type of node comparison.\n");
    }
    return 0;
}