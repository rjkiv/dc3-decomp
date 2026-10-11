#include "meta_ham/AccomplishmentCampaignConditional.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/AccomplishmentConditional.h"
#include "meta_ham/HamProfile.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

AccomplishmentCampaignConditional::AccomplishmentCampaignConditional(DataArray *d, int i)
    : AccomplishmentConditional(d, i) {
    Configure(d);
}

AccomplishmentCampaignConditional::~AccomplishmentCampaignConditional() {}

bool AccomplishmentCampaignConditional::IsFulfilled(HamProfile *profile) const {
    static Symbol crewsong("crewsong");
    static Symbol crewcomplete("crewcomplete");
    static Symbol crewcomplete_count("crewcomplete_count");
    FOREACH (it, m_lConditions) {
        const AccomplishmentCondition &curCond = *it;
        Difficulty curDiff = curCond.m_eDifficulty;
        Symbol era = curCond.m_symValue;
        int val = curCond.m_iValue;
        Symbol condition = curCond.m_symCondition;
        const CampaignProgress &progress = profile->GetCampaignProgress(curDiff);
        if (condition == crewsong) {
            if (progress.IsDanceCrazeSongAvailable(era))
                return true;
        } else if (condition == crewcomplete) {
            if (progress.IsEraComplete(era))
                return true;
        } else if (condition == crewcomplete_count) {
            if (progress.GetNumCompletedEras() >= val) {
                return true;
            }
        } else {
            MILO_NOTIFY("Condition is not currently supported: %s ", condition);
            return false;
        }
    }
    return false;
}

void AccomplishmentCampaignConditional::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x20);
}
