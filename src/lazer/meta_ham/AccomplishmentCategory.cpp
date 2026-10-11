#include "lazer/meta_ham/AccomplishmentCategory.h"
#include "AccomplishmentCategory.h"
#include "lazer/meta_ham/Award.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

AccomplishmentCategory::AccomplishmentCategory(DataArray const *d, int i)
    : m_iIndex(i), m_symName(""), m_symGroup(""), m_symAward("") {
    Configure(d);
}

AccomplishmentCategory::~AccomplishmentCategory() {}

Symbol AccomplishmentCategory::GetName() const { return m_symName; }

Symbol AccomplishmentCategory::GetAward() const { return m_symAward; }

Symbol AccomplishmentCategory::GetGroup() const { return m_symGroup; }

bool AccomplishmentCategory::HasAward() const { return m_symAward != ""; }

void AccomplishmentCategory::Configure(DataArray const *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x1e);
    m_symName = i_pConfig->Sym(0);

    static Symbol group("group");
    i_pConfig->FindData(group, m_symGroup);

    static Symbol award("award");
    i_pConfig->FindData(award, m_symAward, false);
}
