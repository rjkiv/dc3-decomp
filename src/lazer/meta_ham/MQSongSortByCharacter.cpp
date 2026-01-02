#include "MQSongSortByCharacter.h"

#include "MQSongSortNode.h"

MQSongCharCmp::MQSongCharCmp(const char *c, const char *c2) : unk4(c), unk8(c2){}

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

NavListHeaderNode *MQSongSortByCharacter::NewHeaderNode(NavListItemNode *node) const {
    auto cmp = node->GetCmp()->GetMQSongCharCmp();
    MQSongCharCmp *songCharCmp = new MQSongCharCmp(cmp->unk4, cmp->unk8);
    Symbol sym = MakeString("mqheader_%s", cmp->unk8);
    return new MQSongHeaderNode(songCharCmp, sym, true);
}

NavListShortcutNode *MQSongSortByCharacter::NewShortcutNode(NavListItemNode *node) const {
    auto cmp = node->GetCmp()->GetMQSongCharCmp();
    MQSongCharCmp *songCharCmp = new MQSongCharCmp(cmp->unk4, cmp->unk8);
    return new NavListShortcutNode(songCharCmp, cmp->unk8, true);
}