#include "meta_ham/OutfitProvider.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Rand.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "utl/Symbol.h"
#include <cstdio>
#include <cstring>

OutfitProvider::OutfitProvider() {}

OutfitProvider::~OutfitProvider() {}

void OutfitProvider::Text(int, int i_iData, UIListLabel *slot, UILabel *label) const {
    MILO_ASSERT(i_iData < NumData(), 0x4D);
    Symbol outfitSym = DataSymbol(i_iData);
    AppLabel *pAppLabel = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(pAppLabel, 0x52);
    if (slot->Matches("asset")) {
        static Symbol player_present("player_present");
        int otherPlayer = !mPlayerIndex;
        if (TheGameData->Player(mPlayerIndex)->Provider()->Property(player_present)->Int()
                != 1
            && TheGameData->Player(otherPlayer)->Provider()->Property(player_present)->Int()
                != 0) {
            char buffer[32];
            buffer[0] = 0;
            memset(&buffer[1], 0, 31);
            sprintf(buffer, "%s%s", outfitSym.Str(), "_title");
            label->SetTextToken(buffer);
        } else {
            label->SetTextToken(outfitSym);
        }
    } else if (slot->Matches("new")) {
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        if (profile) {
            pAppLabel->SetNew(profile->IsContentNew(outfitSym));
        } else {
            pAppLabel->SetTextToken(gNullStr);
        }
    } else if (slot->Matches("lock")) {
        bool locked = !TheProfileMgr.IsContentUnlocked(outfitSym);
        if (locked) {
            static Symbol is_in_party_mode("is_in_party_mode");
            if (TheHamProvider->Property(is_in_party_mode)->Int()) {
                if (strstr(outfitSym.Str(), "01")) {
                    pAppLabel->SetTextToken(gNullStr);
                }
            } else {
                pAppLabel->SetIcon('B');
            }
        } else {
            pAppLabel->SetTextToken(gNullStr);
        }
    } else {
        label->SetTextToken(slot->GetDefaultText());
    }
}

Symbol OutfitProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0xa4);
    return m_vOutfits[i_iData];
}

bool OutfitProvider::CanSelect(int data) const {
    MILO_ASSERT_RANGE(data, 0, NumData(), 0x9b);
    Symbol s = DataSymbol(data);
    return TheProfileMgr.IsContentUnlocked(s);
}

void OutfitProvider::UpdateList() {
    m_vOutfits.clear();
    HamPlayerData *pPlayerData = TheGameData->Player(mPlayerIndex);
    MILO_ASSERT(pPlayerData, 0x36);
    Symbol character = pPlayerData->Char();
    int numOutfits = GetNumCharacterOutfits(character, false);
    for (int i = 0; i < numOutfits; i++) {
        DataArray *charOutfitEntry = GetCharacterOutfitEntry(character, i);
        Symbol outfit = charOutfitEntry->Sym(0);
        bool hiddenCheck = false;
        charOutfitEntry->FindData("hidden", hiddenCheck, false);
        if (!hiddenCheck || TheProfileMgr.IsContentUnlocked(outfit)) {
            m_vOutfits.push_back(outfit);
        }
    }
}

Symbol OutfitProvider::GetRandomAvailableOutfit() const {
    std::vector<Symbol> vOutfits;
    int num = NumData();
    vOutfits.reserve(num);
    for (int i = 0; i < num; i++) {
        if (CanSelect(i)) {
            vOutfits.push_back(DataSymbol(i));
        }
    }
    return vOutfits[RandomInt(0, vOutfits.size())];
}
