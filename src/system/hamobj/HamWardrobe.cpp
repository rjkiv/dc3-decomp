#include "hamobj/HamWardrobe.h"
#include "char/CharDriver.h"
#include "char/CharInterest.h"
#include "char/Character.h"
#include "char/FileMerger.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamGameData.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Overlay.h"
#include "rndobj/Wind.h"
#include "utl/Symbol.h"

HamWardrobe *TheHamWardrobe;

HamWardrobe::HamWardrobe()
    : mCrowdMembers(this), mMainCharacters(this, (EraseMode)1, kObjListAllowNull),
      unk34("medium"), unk38(0), unk3c(gNullStr), unk40(0) {
    static DataNode &n = DataVariable("hamwardrobe");
    if (TheHamWardrobe) {
        MILO_NOTIFY("Trying to make > 1 HamWardrobe, which should be single");
    }
    n = this;
    TheHamWardrobe = this;
    for (int i = 0; i < 2; i++) {
        mMainCharacters.push_back(nullptr);
    }
    mOverlay = RndOverlay::Find("crowd_groups", false);
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
    SYNC_PROP_SET(overlay_enabled, mOverlay->Showing(), mOverlay->SetShowing(_val.Int()))
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamWardrobe)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << unk34;
END_SAVES

BEGIN_COPYS(HamWardrobe)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(HamWardrobe)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(unk34)
    END_COPYING_MEMBERS
END_COPYS

void HamWardrobe::SetBackupOverrideOutfits(Symbol s1, Symbol s2) {
    unk4c[0] = s1;
    unk4c[1] = s2;
}

namespace {
    Symbol HandleRobot(Symbol s) {
        static Symbol robota01("robota01");
        static Symbol robota02("robota02");
        static Symbol robotb01("robotb01");
        static Symbol robotb02("robotb02");
        if (s == robota02) {
            s = robota01;
        }
        if (s == robotb02) {
            s = robotb01;
        }
        return s;
    }
}

Symbol HamWardrobe::GetBackupOutfitOverride(int x) {
    if (x >= 0 && x < 2) {
        return unk4c[x];
    } else
        return gNullStr;
}

Symbol HamWardrobe::GetCrewChar(Symbol s, int i) {
    return DataGetMacro("CREWS")->FindArray(s, "characters")->Sym(i + 1);
}

Symbol GetOutfitBackupDancer(Symbol outfit) {
    MILO_ASSERT(!outfit.Null(), 0x112);
    DataArray *entry = GetOutfitEntry(outfit, true);
    static Symbol backup_dancers("backup_dancers");
    DataArray *backupArr = entry->FindArray(backup_dancers, true);
    return backupArr->Sym(1);
}

Symbol GetDanceBattleBackupOutfit(Symbol s1, Symbol s2) {
    DataArray *charArr = DataGetMacro("CREWS")->FindArray(s2, "characters");
    Symbol out(gNullStr);
    String str88(s1);
    String str90(str88);
    if (str90.length() >= 2) {
        str90 = str90.substr(0, str90.length() - 2);
    }
    for (int i = 1; i < charArr->Size(); i++) {
    }
    return out;
}

HamCharacter *HamWardrobe::GetBackup(int i) const {
    return Dir()->Find<HamCharacter>(MakeString("backup%d", i), false);
}

void HamWardrobe::EndCrowdOverride() {
    if (unk38) {
        if (unk3c == gNullStr) {
            unk38 = false;
            int flags = unk40;
            if (flags & 2) {
                flags &= ~2;
            }
            PlayCrowdAnimation(unk44, flags, false);
        }
    }
}

void HamWardrobe::ForceCrowdAnimationEnd() {
    unk3c = gNullStr;
    EndCrowdOverride();
}

void HamWardrobe::ForceCrowdAnimationStart(Symbol s) {
    static Symbol none("none");
    if (s == gNullStr || s == none || s == unk3c) {
        if (unk3c != gNullStr) {
            ForceCrowdAnimationEnd();
        }
    } else {
        MILO_LOG(
            "HamWardrobe::ForceCrowdAnimationStart: %s : current mCrowdForceState = '%s'\n",
            s.Str(),
            unk3c.Str()
        );
        unk3c = gNullStr;
        PlayCrowdAnimation(s, 1, true);
        unk3c = s;
        static Symbol none2("none");
        if (s == none2) {
            unk3c = gNullStr;
        }
    }
}

HamCharacter *HamWardrobe::LoadMainCharacter(int index, Symbol s, bool b3) {
    MILO_ASSERT(index < mMainCharacters.size(), 0x160);
    HamCharacter *c = mMainCharacters[index];
    c->SetOutfit(s);
    c->StartLoad(b3);
    return c;
}

void HamWardrobe::LoadCrowdClips(Symbol s1, Symbol s2, bool b3) {
    FileMerger *fm = Dir()->Find<FileMerger>("crowd_clips.fm", false);
    if (fm) {
        static Message msg("load_tempo", 0, 0, 0, 0);
        msg[0] = s1;
        msg[1] = b3;
        msg[2] = s2;
        fm->HandleType(msg);
    }
}

bool HamWardrobe::AllCharsLoaded() {
    for (int i = 0; i < 2; i++) {
        HamCharacter *c = mMainCharacters[i];
        if (c && c->IsLoading())
            return false;
    }
    HamCharacter *c = GetBackup(0);
    int i = 1;
    for (; c != nullptr; c = GetBackup(i++)) {
        if (c->IsLoading())
            return false;
    }
    FileMerger *fm = Dir()->Find<FileMerger>("crowd_clips.fm", false);
    if (fm && fm->HasPendingFiles())
        return false;
    else
        return true;
}

HamCharacter *HamWardrobe::GetCharacter(int i) const {
    MILO_ASSERT((0) <= (i) && (i) < (2), 0x213);
    return (HamCharacter *)mMainCharacters[i];
}

void HamWardrobe::ClearCrowdClips() { LoadCrowdClips(gNullStr, gNullStr, false); }

void HamWardrobe::ClearCrowd() {
    mCrowdMembers.clear();
    unk3c = gNullStr;
    unk38 = false;
}

void HamWardrobe::SyncInterestObjects(ObjectDir *dir) {
    ObjPtrList<CharInterest> interests(this);
    for (ObjDirItr<CharInterest> it(dir, true); it != nullptr; ++it) {
        interests.push_back(it);
    }
    for (ObjDirItr<Character> it(dir, true); it != nullptr; ++it) {
        for (ObjDirItr<CharInterest> cit(it, true); cit != nullptr; ++cit) {
            interests.push_back(cit);
        }
    }
    for (int i = 0; i < 2; i++) {
        HamCharacter *c = mMainCharacters[i];
        if (c) {
            c->SetInterestObjects(interests, nullptr);
        }
    }
    HamCharacter *c = GetBackup(0);
    int i = 1;
    for (; c != nullptr; c = GetBackup(i++)) {
        c->SetInterestObjects(interests, nullptr);
    }
}

void HamWardrobe::UpdateOverlay() {
    if (mOverlay && mOverlay->Showing()) {
        for (ObjPtrList<Character>::iterator it = mCrowdMembers.begin();
             it != mCrowdMembers.end();
             ++it) {
            Character *cur = *it;
            if (cur) {
                *mOverlay << cur->Name() << ": ";
                CharDriver *driver = cur->Driver();
                if (driver) {
                }
            }
        }
    }
}

void HamWardrobe::SetDir(ObjectDir *dir) {
    RndWind *wind = dir->Find<RndWind>("world.wind", false);
    if (wind) {
        wind->SetWindOwner(dir->Find<RndWind>("wind.wind", false));
    }
    HamCharacter *c = GetBackup(0);
    int i = 1;
    for (; c != nullptr; c = GetBackup(i++)) {
        c->SetFocusInterest(nullptr, 0);
        c->EnableBlinks(true, false);
    }
    for (int i = 0; i < 2; i++) {
        HamCharacter *c = mMainCharacters[i];
        if (c) {
            c->SetFocusInterest(nullptr, 0);
            c->EnableBlinks(true, false);
        }
    }
    SyncInterestObjects(dir);
}
