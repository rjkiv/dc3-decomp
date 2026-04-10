#include "lazer/meta_ham/AccomplishmentOneShot.h"
#include "AccomplishmentOneShot.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamPlayerData.h"
#include "lazer/meta_ham/AccomplishmentConditional.h"
#include "meta_ham/Accomplishment.h"
#include "meta_ham/AccomplishmentProgress.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

AccomplishmentOneShot::AccomplishmentOneShot(DataArray *d, int i)
    : AccomplishmentConditional(d, i) {
    Configure(d);
}

AccomplishmentOneShot::~AccomplishmentOneShot() {}

bool AccomplishmentOneShot::AreOneShotConditionsMet(
    HamPlayerData *hpd, HamProfile *profile, Symbol shortname, Difficulty d
) {
    static Symbol stars("stars");
    static Symbol flawless_a("flawless_a");
    static Symbol flawless_b("flawless_b");
    static Symbol nices_a("nices_a");
    static Symbol nices_b("nices_b");
    static Symbol days("days");
    static Symbol weekends("weekends");
    static Symbol hardest_stars("hardest_stars");
    const AccomplishmentProgress &progress = profile->GetAccomplishmentProgress();
    FOREACH (it, m_lConditions) {
        Symbol condition = it->mCondition;
        Difficulty diffForCondition = it->mDifficulty;
        int targetValueForCondition = it->mValue;
        bool eligible;
        if (diffForCondition == kDifficultyBeginner) {
            eligible = true;
        } else if (d == kDifficultyBeginner) {
            eligible = false;
        } else if (diffForCondition <= d) {
            eligible = true;
        } else {
            eligible = false;
        }
        if (eligible) {
            if (condition == stars) {
                static Symbol stars_earned("stars_earned");
                const DataNode *pStarsNode =
                    TheHamProvider->Property(stars_earned, false);
                MILO_ASSERT(pStarsNode, 0x112);
                if (pStarsNode->Int() >= targetValueForCondition) {
                    return true;
                }
            } else if (condition == flawless_a || condition == flawless_b) {
                if (progress.GetFlawlessMoveCount() >= targetValueForCondition) {
                    return true;
                }
            } else if (condition == nices_a || condition == nices_b) {
                if (progress.GetNiceMoveCount() >= targetValueForCondition) {
                    return true;
                }
            } else if (condition == days) {
                if (progress.NumDays() >= targetValueForCondition) {
                    return true;
                }
            } else if (condition == weekends) {
                if (progress.NumWeekends() >= targetValueForCondition) {
                    return true;
                }
            } else if (condition == hardest_stars) {
                static Symbol omg("omg");
                if (shortname == omg) {
                    static Symbol stars_earned("stars_earned");
                    const DataNode *pStarsNode =
                        TheHamProvider->Property(stars_earned, false);
                    MILO_ASSERT(pStarsNode, 0x14C);
                    if (pStarsNode->Int() >= targetValueForCondition) {
                        return true;
                    }
                }
            } else {
                MILO_NOTIFY("Condition is not currently supported: %s ", condition);
                return false;
            }
        }
    }
    return false;
}

void AccomplishmentOneShot::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x23);
}
