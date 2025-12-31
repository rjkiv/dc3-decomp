#include "SongSortByLocation.h"

#include "CampaignEra.h"
#include "SongRecord.h"
#include "SongSortNode.h"
#include "meta/Sorting.h"

int ConvertGameOriginSymbolToEnum(Symbol sym) {
    static Symbol ham1("ham1");
    static Symbol ham2("ham2");
    static Symbol ham3("ham3");
    if (sym == ham1) return 0;
    if (sym == ham2) return 1;
    if (sym == ham3) return 2;
    else return 3;
}

int LocationCmp::Compare(const NavListItemSortCmp *cmp, NavListNodeType type) const {
    switch (type) {
        case kNodeHeader: {
            const LocationCmp *locCmp = cmp->GetLocationCmp();
            int a = ConvertGameOriginSymbolToEnum(mLocation);
            int b = ConvertGameOriginSymbolToEnum(locCmp->mLocation);
            return a - b;
        }
        case kNodeItem: {
            const LocationCmp *locCmp = cmp->GetLocationCmp();
            static Symbol ham1("ham1");
            static Symbol ham2("ham2");
            static Symbol dlc("dlc");
            int a = ConvertGameOriginSymbolToEnum(mLocation);
            int b = ConvertGameOriginSymbolToEnum(locCmp->mLocation);
            if (a == b) return AlphaKeyStrCmp(mName, locCmp->mName, false);
            else return a - b;
        }
        case kNodeShortcut:
            return 0;

        default:
            MILO_FAIL("invalid type of node comparison.\n");
    }
    return 0;
}

NavListShortcutNode *SongSortByLocation::NewShortcutNode(NavListItemNode *itemNode) const {
    SongSortNode *ssNode = dynamic_cast<SongSortNode *>(itemNode);
    //auto a = ssNode->;
    return 0;
}

NavListItemNode *SongSortByLocation::NewItemNode(void *v) const {
    auto movie = static_cast<CampaignEra *>(v)->GetIntroMovie();
    NavListItemSortCmp *cmp = 0;
    SongSortNode *ssNode = 0;

    NavListItemNode *ret;
    return ret;
}