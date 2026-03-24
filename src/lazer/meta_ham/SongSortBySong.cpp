#include "SongSortBySong.h"

#include "meta/Sorting.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/SongSortNode.h"
#include "ui/UIListWidget.h"
#include <cstdio>

int SongCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    switch (type) {
    case kNodeShortcut:
        return 0;

    case kNodeHeader: {
        const SongCmp *songCmp = cmp->GetSongCmp();
        int iVar3 = unk4 - songCmp->unk4;
        if (unk8 == 0)
            return iVar3;
        if (0 < iVar3)
            return iVar3;
        return unk8 - songCmp->unk4 >> 31 & unk8 - songCmp->unk4; // something strange
                                                                  // here
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

NavListHeaderNode *SongSortBySong::NewHeaderNode(NavListItemNode *p1) const {
    SongSortNode *node = dynamic_cast<SongSortNode *>(p1);
    const char *title = node->Record()->Metadata()->Title();
    SongCmp *cmp = new SongCmp(title, nullptr);

    char sortLetter[2] = { 0, 0 };
    sortLetter[0] = title[0];
    Symbol sortSym(sortLetter);

    return new SongHeaderNode(cmp, sortSym, true);
}

NavListShortcutNode *SongSortBySong::NewShortcutNode(NavListItemNode *p1) const {
    SongSortNode *node = dynamic_cast<SongSortNode *>(p1);
    const char *title = node->Record()->Metadata()->Title();
    SongCmp *cmp = new SongCmp(title, nullptr);

    char sortLetter[2] = { 0, 0 };
    sortLetter[0] = title[0];

    Symbol sortSym(sortLetter);

    return new NavListShortcutNode(cmp, sortSym, true);
}

NavListItemNode *SongSortBySong::NewItemNode(void *p1) const {
    SongRecord *record = static_cast<SongRecord *>(p1);
    const char *title = record->Metadata()->Title();
    SongCmp *cmp = new SongCmp(title, nullptr);

    return new SongSortNode(cmp, record);
}

NavListHeaderNode *
SongSortBySong::NewHeaderNode(NavListItemNode *node, NavListItemNode *node2) const {
    SongSortNode *sortNode1 = dynamic_cast<SongSortNode *>(node);
    SongSortNode *sortNode2 = dynamic_cast<SongSortNode *>(node2);

    const char *title1 = sortNode1->Record()->Metadata()->Title();
    const char *title2 = sortNode2->Record()->Metadata()->Title();

    SongCmp *cmp = new SongCmp(title1, title2);

    char buf1[2] = { 0, 0 };
    buf1[0] = title1[0];

    char buf2[2] = { 0, 0 };
    buf2[0] = title2[0];

    char title[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    if (*title1 == *title2) {
        sprintf(title, "%s", buf1);
    } else {
        sprintf(title, "%s - %s", buf1, buf2);
    }
    Symbol s = title;
    return new SongHeaderNode(cmp, s, true);
}
