#include "hamobj/HamWardrobe.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Overlay.h"
#include "utl/Symbol.h"

HamWardrobe *TheHamWardrobe;

HamWardrobe::HamWardrobe()
    : mCrowdMembers(this), unk18(this, (EraseMode)1, kObjListAllowNull), unk34("medium"),
      unk38(0), unk3c(gNullStr), unk40(0) {
    static DataNode &n = DataVariable("hamwardrobe");
    if (TheHamWardrobe) {
        MILO_NOTIFY("Trying to make > 1 HamWardrobe, which should be single");
    }
    n = this;
    TheHamWardrobe = this;
    for (int i = 0; i < 2; i++) {
        unk18.push_back(nullptr);
    }
    unk54 = RndOverlay::Find("crowd_groups", false);
}

HamWardrobe::~HamWardrobe() {
    if (TheHamWardrobe == this) {
        static DataNode &n = DataVariable("hamwardrobe");
        n = NULL_OBJ;
        TheHamWardrobe = nullptr;
    }
}

BEGIN_HANDLERS(HamWardrobe)
    HANDLE(set_venue, OnSetVenue)
    HANDLE_EXPR(chars_dir, Dir())
    HANDLE_EXPR(get_character, GetCharacter(_msg->Int(2)))
    HANDLE_EXPR(get_backup, GetBackup(_msg->Int(2)))
    HANDLE(add_crowd, OnAddCrowd)
    HANDLE_ACTION(set_force_character, unk48 = _msg->Sym(2))
    HANDLE_ACTION(crowd, PlayCrowdAnimation(_msg->Sym(2), 1, false))
    HANDLE_ACTION(crowd_end_override, EndCrowdOverride())
    HANDLE_ACTION(crowd_force_state_enable, ForceCrowdAnimationStart(_msg->Sym(2)))
    HANDLE_ACTION(crowd_force_state_disable, ForceCrowdAnimationEnd())
    HANDLE_EXPR(get_crew_char, GetCrewChar(_msg->Sym(2), _msg->Int(3)))
    HANDLE(load_characters, OnLoadCharacters)
    HANDLE_ACTION(
        set_backup_override_outfits, SetBackupOverrideOutfits(_msg->Sym(2), _msg->Sym(3))
    )
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamWardrobe)
    SYNC_PROP(crowd_members, mCrowdMembers)
    SYNC_PROP_SET(overlay_enabled, unk54->Showing(), unk54->SetShowing(_val.Int()))
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void HamWardrobe::SetBackupOverrideOutfits(Symbol s1, Symbol s2) {
    unk4c[0] = s1;
    unk4c[1] = s2;
}
