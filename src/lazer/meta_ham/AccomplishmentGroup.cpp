#include "lazer/meta_ham/AccomplishmentGroup.h"
#include "AccomplishmentGroup.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

AccomplishmentGroup::AccomplishmentGroup(DataArray *d, int i)
    : m_symName(""), m_iIndex(i), m_symAward("") {
    Configure(d);
}

AccomplishmentGroup::~AccomplishmentGroup() {}

bool AccomplishmentGroup::HasAward() const { return m_symAward != ""; }

Symbol AccomplishmentGroup::GetName() const { return m_symName; }

void AccomplishmentGroup::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x1b);
    m_symName = i_pConfig->Sym(0);

    static Symbol award("award");
    i_pConfig->FindData(award, m_symAward, false);
}
