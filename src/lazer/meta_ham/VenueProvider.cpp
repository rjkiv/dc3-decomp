#include "meta_ham/VenueProvider.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/System.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"
#include <cstdio>

VenueProvider::VenueProvider() : mPlayerIndex(0) {}
VenueProvider::~VenueProvider() {}

void VenueProvider::Text(int, int data, UIListLabel *uiListLabel, UILabel *uiLabel) const {
    MILO_ASSERT_RANGE(data, 0, mVenues.size(), 0x4e);
    Symbol venueSym = DataSymbol(data);
    if (uiListLabel->Matches("venue")) {
        static Symbol player_present("player_present");
        int otherPlayer = !mPlayerIndex;
        if (TheGameData->Player(mPlayerIndex)->Provider()->Property(player_present)->Int()
                != 1
            && TheGameData->Player(otherPlayer)->Provider()->Property(player_present)->Int()
                != 0) {
            char buffer[32];
            buffer[0] = 0;
            memset(&buffer[1], 0, 31);
            sprintf(buffer, "%s%s", venueSym.Str(), "_title");
            uiLabel->SetTextToken(buffer);
        } else {
            uiLabel->SetTextToken(venueSym);
        }
    } else if (uiListLabel->Matches("lock")) {
        if (!TheProfileMgr.IsContentUnlocked(venueSym)) {
            uiLabel->SetIcon('B');
        } else {
            uiLabel->SetTextToken(gNullStr);
        }
    }
}

Symbol VenueProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mVenues.size(), 0x82);
    return mVenues[idx];
}

void VenueProvider::UpdateList() {
    mVenues.clear();
    static Symbol Default("default"); // didnt like me using a lowercase d
    mVenues.push_back(Default);
    DataArray *venueArray = SystemConfig()->FindArray("venues", false);
    if (venueArray)
        for (int i = 1; i < venueArray->Size(); i++) {
            DataArray *pVenueEntryArray = venueArray->Node(i).Array(venueArray);
            MILO_ASSERT(pVenueEntryArray, 0x2f);
            Symbol s = pVenueEntryArray->Sym(0);
            static Symbol never_show("never_show");
            bool neverShowCheck = false;
            pVenueEntryArray->FindData(never_show, neverShowCheck, false);
            if (!neverShowCheck) {
                static Symbol hidden("hidden");
                bool hiddenCheck = false;
                pVenueEntryArray->FindData(hidden, hiddenCheck, false);
                if (TheProfileMgr.IsContentUnlocked(s) || !hiddenCheck) {
                    mVenues.push_back(s);
                }
            }
        }
}
