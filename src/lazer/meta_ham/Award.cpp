#include "lazer/meta_ham/Award.h"
#include "meta_ham/AccomplishmentManager.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

Award::Award(DataArray *d, int i)
    : mName(""), unk8(i), mIsSecret(false), mIsSilent(false), mArt(gNullStr) {
    Configure(d);
}

Award::~Award() { mAwardEntries.clear(); }

const char *Award::GetArt() const {
    return MakeString("ui/award/award_art/%s_keep.png", mArt.Str());
}

void Award::GrantAwards(class HamProfile *hp) {
    FOREACH (it, mAwardEntries) {
        GrantAward(*it, hp);
    };
}

Symbol Award::GetDisplayName() const {
    static Symbol asset("asset");
    Symbol name = mName;
    if (!mAwardEntries.empty()) {
        if (mAwardEntries.size() == 1) {
            const AwardEntry &entry = mAwardEntries.front();
            if (entry.m_symAwardCategory == asset) {
                name = entry.m_symAward;
            }
        }
    }
    return name;
}

void Award::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x21);
    mName = i_pConfig->Sym(0);
    static Symbol is_secret("is_secret");
    i_pConfig->FindData(is_secret, mIsSecret, false);
    static Symbol is_silent("is_silent");
    i_pConfig->FindData(is_silent, mIsSilent, false);
    static Symbol art("art");
    i_pConfig->FindData(art, mArt, false);
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
                TheAccomplishmentMgr->AddAssetAward(entry.m_symAward, mName);
            }
            mAwardEntries.push_back(entry);
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
