#include "meta_ham/AccomplishmentCountConditional.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/AccomplishmentConditional.h"
#include "meta_ham/AccomplishmentProgress.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/MetaPerformer.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

AccomplishmentCountConditional::AccomplishmentCountConditional(DataArray *d, int i)
    : AccomplishmentConditional(d, i) {
    Configure(d);
}

AccomplishmentCountConditional::~AccomplishmentCountConditional() {}

void AccomplishmentCountConditional::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x1c);
}

bool AccomplishmentCountConditional::IsFulfilled(HamProfile *profile) const {
    static Symbol character("character");
    static Symbol stars("stars");
    static Symbol character_no_outfit("character_no_outfit");
    int numStars = 0;
    FOREACH (it, m_lConditions) {
        Symbol condition = it->unk0;
        if (condition == character) {
            MILO_NOTIFY("Character-based achievements aren't supported in turbo (yet)");
        } else if (condition == stars) {
            numStars = it->unk4;
        } else if (condition != character_no_outfit) {
            MILO_NOTIFY("Condition is not currently supported: %s ", condition);
            return false;
        }
    }
    AccomplishmentProgress &progress = profile->AccessAccomplishmentProgress();
    MetaPerformer *pPerformer = MetaPerformer::Current();
    progress.IncrementCount(GetName(), pPerformer->GetUnk38());
    return progress.GetCount(GetName()) >= numStars; // idk what this line is doin
}
