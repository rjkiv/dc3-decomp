#include "SongSortByLocation.h"

#include "CampaignEra.h"
#include "SongRecord.h"
#include "SongSortNode.h"
#include "meta/Sorting.h"
#include "meta_ham/NavListNode.h"

int ConvertGameOriginSymbolToEnum(Symbol sym) {
    static Symbol ham1("ham1");
    static Symbol ham2("ham2");
    static Symbol ham3("ham3");
    if (sym == ham1)
        return 0;
    if (sym == ham2)
        return 1;
    if (sym == ham3)
        return 2;
    else
        return 3;
}

int LocationCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    switch (type) {
    case kNodeHeader: {
        const LocationCmp *locCmp = cmp->GetLocationCmp();
        int a = ConvertGameOriginSymbolToEnum(mOrigin);
        int b = ConvertGameOriginSymbolToEnum(locCmp->mOrigin);
        return a - b;
    }
    case kNodeItem: {
        const LocationCmp *locCmp = cmp->GetLocationCmp();
        static Symbol ham1("ham1");
        static Symbol ham2("ham2");
        static Symbol dlc("dlc");
        int a = ConvertGameOriginSymbolToEnum(mOrigin);
        int b = ConvertGameOriginSymbolToEnum(locCmp->mOrigin);
        if (a == b)
            return AlphaKeyStrCmp(mName, locCmp->mName, false);
        else
            return a - b;
    }
    case kNodeShortcut:
        return 0;

    default:
        MILO_FAIL("invalid type of node comparison.\n");
    }
    return 0;
}

NavListShortcutNode *
SongSortByLocation::NewShortcutNode(NavListItemNode *itemNode) const {
    SongSortNode *sortNode = dynamic_cast<SongSortNode *>(itemNode);
    Symbol location = sortNode->GetSongRecord()->GetData()->GameOrigin();
    const char *name = sortNode->GetSongRecord()->GetData()->Title();
    LocationCmp *newCmp = new LocationCmp(name, location.Str());
    return new NavListShortcutNode(newCmp, location, true);
}

NavListItemNode *SongSortByLocation::NewItemNode(void *v) const {
    SongRecord *record = static_cast<SongRecord *>(v);
    Symbol origin = record->GetData()->GameOrigin();
    const char *name = record->GetData()->Title();
    LocationCmp *cmp = new LocationCmp(name, origin);
    return new SongSortNode(cmp, record);
}

NavListHeaderNode *SongSortByLocation::NewHeaderNode(NavListItemNode *node) const {
    SongSortNode *sortNode = dynamic_cast<SongSortNode *>(node);
    Symbol location = sortNode->GetSongRecord()->GetData()->GameOrigin();
    if (ConvertGameOriginSymbolToEnum(location) == 3) {
        location = Symbol("DLC");
    }
    const char *name = sortNode->GetSongRecord()->GetData()->Title();
    LocationCmp *cmp = new LocationCmp(name, location.Str());
    return new SongHeaderNode(cmp, location, true);
}
