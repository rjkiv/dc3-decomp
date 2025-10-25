#include "lazer/meta_ham/Accomplishment.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

Accomplishment::Accomplishment(DataArray *a, int i)
    : unk4(""), unk14(0), mCategory(""), mAward(""), unk20(gNullStr), unk24(0),
      unk28(gNullStr), unk2c(-1), unk30(false), unk34(-1), unk38(-1), unk3c(-1), unk4c(0),
      unk50(-1), unk54(-1), unk58(true), unk59(false), unk5c(i), unk60(-1), unk64(true),
      unk65(false), isSecondaryGoal(false), unk67(false) {
    Configure(a);
}

Accomplishment::~Accomplishment() {}

bool Accomplishment::ShowBestAfterEarn() const { return unk58; }

bool Accomplishment::CanBeLaunched() const {
    static Symbol acc_calibrate("acc_calibrate");
    static Symbol acc_charactercreate("acc_charactercreate");

    if (unk4 == acc_calibrate) {
        return true;
    } else
        return unk4 == acc_charactercreate;
}

void Accomplishment::UpdateIncrementalEntryName(UILabel *, Symbol) {
    MILO_ASSERT(false, 0x4c);
}

Symbol Accomplishment::GetCategory() const { return mCategory; }

bool Accomplishment::HasGamerpicReward() const { return unk50 != -1; }

bool Accomplishment::HasAvatarAssetReward() const { return unk54 != -1; }

Symbol Accomplishment::GetAward() const { return mAward; }

bool Accomplishment::HasAward() const { return !(mAward == ""); }

bool Accomplishment::IsSecondaryGoal() const { return isSecondaryGoal; }

bool Accomplishment::IsDynamic() const { return !unk40.empty(); }

char const *Accomplishment::GetIconArt() const {
    return MakeString("ui/accomplishments/accomplishment_art/%s_keep.png", unk4.Str());
}

void Accomplishment::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x34);
    unk4 = i_pConfig->Sym(0);

    int launchabledifficulty = 0;
    static Symbol launchable_difficulty("launchable_difficulty");
    if (i_pConfig->FindData(launchable_difficulty, launchabledifficulty, false)) {
        unk24 = launchabledifficulty;
    }

    static Symbol launchable_playercount_min("launchable_playercount_min");
    i_pConfig->FindData(launchable_playercount_min, unk34, false);

    static Symbol launchable_playercount_max("launchable_playercount_max");
    i_pConfig->FindData(launchable_playercount_max, unk38, false);

    static Symbol launchable_requires_unison_ability("launchable_requires_unison_ability");
    i_pConfig->FindData(launchable_requires_unison_ability, unk30, false);

    static Symbol secret_prereqs("secret_prereqs");
    DataArray *preReqs = i_pConfig->FindArray(secret_prereqs, false);
    if (preReqs) {
        for (int i = 1; i < preReqs->Size(); i++) {
            Symbol s = preReqs->Sym(i);
            unk8.push_back(s);
        }
    }

    static Symbol dynamic_prereqs("dynamic_prereqs");
    DataArray *dynPreReqs = i_pConfig->FindArray(dynamic_prereqs, false);
    if (dynPreReqs) {
        static Symbol num_songs("num_songs");
        dynPreReqs->FindData(num_songs, unk3c, false);

        static Symbol songs("songs");
        DataArray *songArray = dynPreReqs->FindArray(songs);
        if (songArray) {
            for (int i = 0; i < songArray->Size(); i++) {
                Symbol s = songArray->Sym(i);
                unk40.push_back(s);
            }
        }
        if (unk40.size() < unk3c) {
            MILO_NOTIFY(
                "There are less songs in the dynamic prereq song list than the num_songs provided: %s",
                unk4.Str()
            );
        }
    }

    static Symbol passive_msg_channel("passive_msg_channel");
    i_pConfig->FindData(passive_msg_channel, unk28, false);

    static Symbol passive_msg_priority("passive_msg_priority");
    i_pConfig->FindData(passive_msg_priority, unk2c, false);

    if (unk28 != gNullStr) {
        if (unk2c < 1) {
            MILO_NOTIFY(
                "Passive Message Priority for goal %s is less than the minimum: %i!",
                unk4.Str(),
                1
            );
            unk2c = 1;
        } else if (1000 < unk2c) {
            MILO_NOTIFY(
                "Passive Message Priority for goal %s is more than the minimum: %i!",
                unk4.Str(),
                1000
            );
            unk2c = 1000;
        }
    }

    static Symbol progress_step("progress_step");
    i_pConfig->FindData(progress_step, unk4c, false);

    static Symbol show_best_after_earn("show_best_after_earn");
    i_pConfig->FindData(show_best_after_earn, unk58, false);

    static Symbol hide_progress("hide_progress");
    i_pConfig->FindData(hide_progress, unk59, false);

    static Symbol can_be_earned_with_no_fail("can_be_earned_with_no_fail");
    i_pConfig->FindData(can_be_earned_with_no_fail, unk64, false);

    static Symbol leaderboard("leaderboard");
    i_pConfig->FindData(leaderboard, unk65, false);

    static Symbol give_to_all("give_to_all");
    i_pConfig->FindData(give_to_all, unk67, false);

    static Symbol xlast_id("xlast_id");
    i_pConfig->FindData(xlast_id, unk60, false);

    static Symbol gamerpic_reward("gamerpic_reward");
    DataArray *gamerPicArray = i_pConfig->FindArray(gamerpic_reward, false);
    if (gamerPicArray) {
        Symbol s = gamerPicArray->Sym(1);
        if (s == nullptr) {
            unk50 = gamerPicArray->Int(0);
        } else {
            MILO_NOTIFY("Invalid gamerpic_reward for %s (data type 0x%x)", unk4, s);
        }
    }

    static Symbol avatarasset_reward("avatarasset_reward");
    DataArray *avatarArray = i_pConfig->FindArray(avatarasset_reward, false);
    if (avatarArray) {
        Symbol s = avatarArray->Sym(1);
        if (s == nullptr) {
            unk54 = avatarArray->Int(0);
        } else {
            MILO_NOTIFY("Invalid avatarasset_reward for %s (data type 0x%x)", unk4, s);
        }
    }

    static Symbol accomplishment_type("accomplishment_type");
    i_pConfig->FindData(accomplishment_type, unk14);

    static Symbol category("category");
    i_pConfig->FindData(category, mCategory);

    static Symbol award("award");
    i_pConfig->FindData(award, mAward, false);

    static Symbol is_secondary_goal("is_secondary_goal");
    i_pConfig->FindData(is_secondary_goal, isSecondaryGoal, false);

    static Symbol units_token("units_token");
    i_pConfig->FindData(units_token, unk20, false);
}
