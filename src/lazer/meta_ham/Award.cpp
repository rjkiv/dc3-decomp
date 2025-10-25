#include "lazer/meta_ham/Award.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

Award::Award(DataArray *d, int i)
    : unk4(""), unk8(i), unkc(false), unkd(false), unk18(gNullStr) {
    Configure(d);
}

Award::~Award() { unk10.clear(); }

char const *Award::GetArt() const {
    return MakeString("ui/award/award_art/%s_keep.png", unk18.Str());
}

void Award::GrantAwards(class HamProfile *hp) {
    FOREACH (it, unk10) {
        GrantAward(*it, hp);
    };
}

Symbol Award::GetDisplayName() const {
    static Symbol asset("asset");

    return 0;
}

void Award::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x21);
    unk4 = i_pConfig->Sym(0);
    static Symbol is_secret("is_secret");
    i_pConfig->FindData(is_secret, unkc, false);

    static Symbol is_silent("is_silent");
    i_pConfig->FindData(is_silent, unkd, false);

    static Symbol art("art");
    i_pConfig->FindData(art, unk18, false);

    static Symbol asset("asset");
    static Symbol awards("awards");
    DataArray *pAwardArray = i_pConfig->FindArray(awards, false);
    if (pAwardArray) {
        MILO_ASSERT(pAwardArray->Size() > 1, 0x35);
        for (int i = 1; i < pAwardArray->Size(); i++) {
            DataArray *pAwardEntryArray = pAwardArray->Array(i);
            MILO_ASSERT(pAwardEntryArray, 0x3a);
            MILO_ASSERT(pAwardEntryArray->Size() == 2, 0x3b);
        }
    }
}

void Award::GrantAward(AwardEntry const &ae, HamProfile *i_pProfile) {
    MILO_ASSERT(i_pProfile, 0x74);
    static Symbol asset("asset");
    if (ae.unk4 == asset) {
        // i_pProfile->UnlockContent(ae.unk4);
    } else {
        MILO_NOTIFY("Award Category is not currently supported: %s ", ae.unk4);
    }
}
