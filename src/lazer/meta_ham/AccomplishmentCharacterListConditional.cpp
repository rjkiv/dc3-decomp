#include "meta_ham/AccomplishmentCharacterListConditional.h"
#include "AccomplishmentCharacterListConditional.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AccomplishmentConditional.h"
#include "meta_ham/AccomplishmentProgress.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

AccomplishmentCharacterListConditional::AccomplishmentCharacterListConditional(
    DataArray *d, int i
)
    : AccomplishmentConditional(d, i) {
    Configure(d);
}

AccomplishmentCharacterListConditional::~AccomplishmentCharacterListConditional() {}

void AccomplishmentCharacterListConditional::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0xce);
    static Symbol charconditions("charconditions");
    DataArray *pCharConditionArray = i_pConfig->FindArray(charconditions);
    if (pCharConditionArray) {
        MILO_ASSERT(pCharConditionArray->Size() > 1, 0xd6);
        for (int i = 0; i < pCharConditionArray->Size(); i++) {
            DataArray *pCharConditionEntryArray = pCharConditionArray->Node(i).Array();
            MILO_ASSERT(pCharConditionEntryArray, 0xdc);
            static Symbol characters("characters");
            static Symbol crews("crews");
            static Symbol oldoutfits("oldoutfits");
            static Symbol unlockableoutfits("unlockableoutfits");

            if (pCharConditionEntryArray->Node(0).Sym(0) == characters) {
                for (int j = 1; j < pCharConditionEntryArray->Size(); j++) {
                    unk70.push_back(pCharConditionEntryArray->Node(j).Sym());
                }
            }
            if (pCharConditionEntryArray->Node(0).Sym(0) == crews) {
                for (int j = 1; j < pCharConditionEntryArray->Size(); j++) {
                    unk7c.push_back(pCharConditionEntryArray->Node(j).Sym());
                }
            }
            if (pCharConditionEntryArray->Node(0).Sym(0) == oldoutfits) {
                for (int j = 1; j < pCharConditionEntryArray->Size(); j++) {
                    unk88.push_back(pCharConditionEntryArray->Node(j).Sym());
                }
            }
            if (pCharConditionEntryArray->Node(0).Sym(0) == unlockableoutfits) {
                for (int j = 1; j < pCharConditionEntryArray->Size(); j++) {
                    unk94.push_back(pCharConditionEntryArray->Node(j).Sym());
                    unka0.push_back(false);
                }
            }
        }
    }
}

bool AccomplishmentCharacterListConditional::AreOldOutfitListConditionsMet() {
    static Symbol is_in_campaign_mode("is_in_campaign_mode");
    if (TheHamProvider->Property(is_in_campaign_mode, true)->Int() == 0) {
        if (TheGameMode->InMode("perform", true)) {
            HamPlayerData *pPlayer1Data = TheGameData->Player(0);
            MILO_ASSERT(pPlayer1Data, 0x82);
            HamPlayerData *pPlayer2Data = TheGameData->Player(1);
            MILO_ASSERT(pPlayer2Data, 0x85);
            Symbol p1crew = pPlayer1Data->Crew();
            Symbol p2crew = pPlayer2Data->Crew();
            if (p1crew != p2crew) {
                MILO_LOG(
                    "crew %s does not match crew %s, achievement conditions not met\n",
                    p1crew,
                    p2crew
                );

            } else {
                bool p1 = false;
                bool p2 = false;
                if (1 < unk88.size()) {
                    for (int i = 0; i < unk88.size(); i++) {
                        if (pPlayer1Data->Outfit() == unk88[i]
                            && pPlayer1Data->IsPlaying()) {
                            p1 = true;
                        }
                        if (pPlayer2Data->Outfit() == unk88[i]
                            && pPlayer2Data->IsPlaying()) {
                            p2 = true;
                        }
                    }
                }
                if (p1 && p2)
                    return true;
            }
        }
    }
    return false;
}
