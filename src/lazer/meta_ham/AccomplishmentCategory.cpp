#include "lazer/meta_ham/AccomplishmentCategory.h"
#include "AccomplishmentCategory.h"
#include "lazer/meta_ham/Award.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

AccomplishmentCategory::AccomplishmentCategory(DataArray const *d, int i)
    : unk4(i), mName(""), mGroup(""), mAward("") {
    Configure(d);
}

AccomplishmentCategory::~AccomplishmentCategory() {}

Symbol AccomplishmentCategory::GetName() const { return mName; }

Symbol AccomplishmentCategory::GetAward() const { return mAward; }

Symbol AccomplishmentCategory::GetGroup() const { return mGroup; }

bool AccomplishmentCategory::HasAward() const { return mAward != ""; }

void AccomplishmentCategory::Configure(DataArray const *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x1e);
    mName = i_pConfig->Sym(0);

    static Symbol group("group");
    i_pConfig->FindData(group, mGroup);

    static Symbol award("award");
    i_pConfig->FindData(award, mAward, false);
}
