#include "lazer/meta_ham/AccomplishmentCampaignConditional.h"
#include "AccomplishmentCampaignConditional.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

AccomplishmentCampaignConditional::AccomplishmentCampaignConditional(DataArray *d, int i)
    : AccomplishmentConditional(d, i) {
    Configure(d);
}

AccomplishmentCampaignConditional::~AccomplishmentCampaignConditional() {}

bool AccomplishmentCampaignConditional::IsFulfilled(HamProfile *) const {
    static Symbol crewsong("crewsong");
    static Symbol crewcomplete("crewcomplete");
    static Symbol crewcomplete_count("crewcomplete_count");

    return false;
}

void AccomplishmentCampaignConditional::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x20);
}
