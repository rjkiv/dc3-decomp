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
    HamPlayerData *hpd, HamProfile *profile, Symbol s, Difficulty d
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
        const AccomplishmentCondition &cur = *it;
        Symbol condition = cur.mCondition;
        Difficulty d2 = cur.mDifficulty;
        int val = cur.mValue;
        unsigned char b6;
        if (d2 == kDifficultyBeginner) {
            b6 = 1;
        } else if (d == kDifficultyBeginner) {
            b6 = 0;
        } else {
            b6 = d2 <= d;
        }
        if (b6 != 0) {
            if (condition == stars) {
                static Symbol stars_earned("stars_earned");
                const DataNode *pStarsNode =
                    TheHamProvider->Property(stars_earned, false);
                MILO_ASSERT(pStarsNode, 0x112);
                if (pStarsNode->Int() >= val)
                    return true;
            } else if (condition == flawless_a) {
                if (progress.GetFlawlessMoveCount() >= val)
                    return true;
            } else if (condition == flawless_b) {
                if (progress.GetFlawlessMoveCount() >= val)
                    return true;
            } else if (condition == nices_a) {
                if (progress.GetNiceMoveCount() >= val)
                    return true;
            } else if (condition == nices_b) {
                if (progress.GetNiceMoveCount() >= val)
                    return true;
            } else if (condition == days) {
                if (progress.NumDays() >= val)
                    return true;
            } else if (condition == weekends) {
                if (progress.NumWeekends() >= val)
                    return true;
            } else if (condition == hardest_stars) {
                static Symbol omg("omg");
                if (s == omg) {
                    static Symbol stars_earned("stars_earned");
                    const DataNode *pStarsNode =
                        TheHamProvider->Property(stars_earned, false);
                    MILO_ASSERT(pStarsNode, 0x14C);
                    if (pStarsNode->Int() >= val)
                        return true;
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
