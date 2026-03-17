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
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/System.h"
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
        for (int i = 1; i < pCharConditionArray->Size(); i++) {
            DataArray *pCharConditionEntryArray = pCharConditionArray->Node(i).Array();
            MILO_ASSERT(pCharConditionEntryArray, 0xdc);
            static Symbol characters("characters");
            static Symbol crews("crews");
            static Symbol oldoutfits("oldoutfits");
            static Symbol unlockableoutfits("unlockableoutfits");

            if (pCharConditionEntryArray->Node(0).Sym(0) == characters) {
                for (int j = 1; j < pCharConditionEntryArray->Size(); j++) {
                    Symbol sym = pCharConditionEntryArray->Node(j).Sym();
                    unk70.push_back(sym);
                }
            }
            if (pCharConditionEntryArray->Node(0).Sym(0) == crews) {
                for (int j = 1; j < pCharConditionEntryArray->Size(); j++) {
                    Symbol sym = pCharConditionEntryArray->Node(j).Sym();
                    unk7c.push_back(sym);
                }
            } else if (pCharConditionEntryArray->Node(0).Sym(0) == oldoutfits) {
                for (int j = 1; j < pCharConditionEntryArray->Size(); j++) {
                    Symbol sym = pCharConditionEntryArray->Node(j).Sym();
                    unk88.push_back(sym);
                }
            } else if (pCharConditionEntryArray->Node(0).Sym(0) == unlockableoutfits) {
                for (int j = 1; j < pCharConditionEntryArray->Size(); j++) {
                    Symbol sym = pCharConditionEntryArray->Node(j).Sym();
                    unk94.push_back(sym);
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
                        Symbol curOutfit = unk88[i];
                        if (pPlayer1Data->Outfit() == curOutfit
                            && pPlayer1Data->IsPlaying()) {
                            p1 = true;
                        }
                        if (pPlayer2Data->Outfit() == curOutfit
                            && pPlayer2Data->IsPlaying()) {
                            p2 = true;
                        }
                    }
                    if (p1 && p2)
                        return true;
                }
            }
        }
    }
    return false;
}

bool AccomplishmentCharacterListConditional::AreCharacterListConditionsMet(
    Symbol s1, HamPlayerData *hpd, HamProfile *profile
) {
    static Symbol dance_use_count("dance_use_count");
    static Symbol char_birthday("char_birthday");
    static Symbol crew_use_count("crew_use_count");
    const AccomplishmentProgress &progress = profile->GetAccomplishmentProgress();
    FOREACH (it, m_lConditions) {
        AccomplishmentCondition &curCond = *it;
        Symbol curSym = curCond.unk0;
        int cur4 = curCond.unk4;
        if (curSym == dance_use_count) {
            for (int i = 0; i < unk70.size(); i++) {
                if (progress.GetCharacterUseCount(unk70[i]) >= cur4) {
                    return true;
                }
            }
        } else if (curSym == char_birthday) {
            static Symbol indaclub("indaclub");
            static Symbol birthday("birthday");
            if (s1 == indaclub) {
                DataArray *birthdayCfg = SystemConfig(birthday);
                DateTime dt;
                GetDateAndTime(dt);
                for (int i = 0; i < unk70.size(); i++) {
                    Symbol cur = unk70[i];
                    if (hpd->Char() == cur) {
                        DataArray *arr = birthdayCfg->FindArray(birthday)->FindArray(cur);
                        if (arr->Node(1).Int() == dt.mMonth + 1) {
                            if (arr->Node(2).Int() == dt.mDay) {
                                return true;
                            }
                        }
                    }
                }
            }
        } else if (curSym == crew_use_count) {
            int i5 = 0;
            for (int i = 0; i < unk7c.size(); i++) {
                i5 += progress.GetCharacterUseCount(unk7c[i]);
            }
            if (i5 >= cur4) {
                return true;
            }
        } else {
            MILO_NOTIFY("Condition is not currently supported: %s ", curSym);
            return false;
        }
    }
    return false;
}

bool AccomplishmentCharacterListConditional::AreUnlockableOutfitListConditionsMet(
    HamPlayerData *hpd, HamProfile *profile
) {
    if (unk94.size() > 1) {
        for (int i = 0; i < unk94.size(); i++) {
            if (hpd->Outfit() == unk94[i] && hpd->IsPlaying()) {
                if (!unka0[i]) {
                    unka0[i] = true;
                    profile->SetUnk32C(profile->GetUnk32C() + 1);
                }
                break;
            }
        }
        if (profile->GetUnk32C() == unk94.size()) {
            return true;
        }
    }
    return false;
}
