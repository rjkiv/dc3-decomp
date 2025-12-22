#include "meta_ham/OutfitProvider.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

OutfitProvider::OutfitProvider() {}

OutfitProvider::~OutfitProvider() {}

bool OutfitProvider::CanSelect(int data) const {
    MILO_ASSERT_RANGE(data, 0, NumData(), 0x9b);
    Symbol s = DataSymbol(data);
    return TheProfileMgr.IsContentUnlocked(s);
}

Symbol OutfitProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0xa4);
    return mOutfits[i_iData];
}

void OutfitProvider::UpdateList() {
    mOutfits.clear();
    HamPlayerData *pPlayerData = TheGameData->Player(unk30);
    MILO_ASSERT(pPlayerData, 0x36);
    Symbol character = pPlayerData->Char();
    int numOutfits = GetNumCharacterOutfits(character, false);
    for (int i = 0; i < numOutfits; i++) {
        DataArray *charOutfitEntry = GetCharacterOutfitEntry(character, i);
        Symbol outfit = charOutfitEntry->Sym(0);
        bool hiddenCheck = false;
        charOutfitEntry->FindData("hidden", hiddenCheck, false);
        if (!hiddenCheck || TheProfileMgr.IsContentUnlocked(outfit)) {
            mOutfits.push_back(outfit);
        }
    }
}
