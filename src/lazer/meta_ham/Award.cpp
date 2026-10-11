#include "lazer/meta_ham/Award.h"
#include "meta_ham/AccomplishmentManager.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

Award::Award(DataArray *d, int i)
    : m_symName(""), m_iIndex(i), m_bIsSecret(false), m_bIsSilent(false),
      m_symArt(gNullStr) {
    Configure(d);
}

Award::~Award() { m_lAwards.clear(); }

const char *Award::GetArt() const {
    return MakeString("ui/award/award_art/%s_keep.png", m_symArt.Str());
}

void Award::GrantAwards(class HamProfile *hp) {
    FOREACH (it, m_lAwards) {
        GrantAward(*it, hp);
    };
}

Symbol Award::GetDisplayName() const {
    static Symbol asset("asset");
    Symbol name = m_symName;
    if (!m_lAwards.empty()) {
        if (m_lAwards.size() == 1) {
            const AwardEntry &entry = m_lAwards.front();
            if (entry.m_symAwardCategory == asset) {
                name = entry.m_symAward;
            }
        }
    }
    return name;
}

void Award::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x21);
    m_symName = i_pConfig->Sym(0);
    static Symbol is_secret("is_secret");
    i_pConfig->FindData(is_secret, m_bIsSecret, false);
    static Symbol is_silent("is_silent");
    i_pConfig->FindData(is_silent, m_bIsSilent, false);
    static Symbol art("art");
    i_pConfig->FindData(art, m_symArt, false);
    static Symbol asset("asset");
    static Symbol awards("awards");
    DataArray *pAwardArray = i_pConfig->FindArray(awards, false);
    if (pAwardArray) {
        MILO_ASSERT(pAwardArray->Size() > 1, 0x35);
        for (int i = 1; i < pAwardArray->Size(); i++) {
            DataArray *pAwardEntryArray = pAwardArray->Node(i).Array();
            MILO_ASSERT(pAwardEntryArray, 0x3a);
            MILO_ASSERT(pAwardEntryArray->Size() == 2, 0x3b);
            AwardEntry entry;
            entry.m_symAwardCategory = pAwardEntryArray->Node(0).Sym();
            entry.m_symAward = pAwardEntryArray->Node(1).Sym();
            if (entry.m_symAwardCategory == asset) {
                TheAccomplishmentMgr->AddAssetAward(entry.m_symAward, m_symName);
            }
            m_lAwards.push_back(entry);
        }
    }
}

void Award::GrantAward(AwardEntry const &ae, HamProfile *i_pProfile) {
    MILO_ASSERT(i_pProfile, 0x74);
    static Symbol asset("asset");
    Symbol award = ae.m_symAwardCategory;
    Symbol awardSym = ae.m_symAward;
    if (award == asset) {
        i_pProfile->UnlockContent(awardSym);
    } else {
        MILO_NOTIFY("Award Category is not currently supported: %s ", award);
    }
}
