#include "meta_ham/VenueProvider.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/System.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"

VenueProvider::VenueProvider() : unk30(0) {}

Symbol VenueProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mVenues.size(), 0x82);
    return mVenues[idx];
}

void VenueProvider::Text(int, int data, UIListLabel *uiListLabel, UILabel *) const {
    MILO_ASSERT_RANGE(data, 0, mVenues.size(), 0x4e);
    Symbol ds = DataSymbol(data);
    if (uiListLabel->Matches("venue")) {
        static Symbol player_present("player_present");
        HamPlayerData *pPlayer = TheGameData->Player(unk30);
    }
}

void VenueProvider::UpdateList() {
    mVenues.clear();
    static Symbol Default("default"); // didnt like me using a lowercase d
    mVenues.push_back(Default);
    DataArray *sysConfig = SystemConfig("venues");
    for (int i = 1; i < sysConfig->Size(); i++) {
        DataArray *pVenueEntryArray = sysConfig->Array(i);
        MILO_ASSERT(pVenueEntryArray, 0x2f);
        Symbol s = pVenueEntryArray->Sym(0);
        static Symbol never_show("never_show");
        if (!pVenueEntryArray->FindData(never_show, unk30, false)) {
            static Symbol hidden("hidden");
            if (TheProfileMgr.IsContentUnlocked(s)) {
                mVenues.push_back(s);
            }
        }
    }
}
