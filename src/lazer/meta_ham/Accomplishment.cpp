#include "lazer/meta_ham/Accomplishment.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

Accomplishment::Accomplishment(DataArray *a, int i)
    : mAccomplishment_Name(""), mAccomplishment_Type(0), mCategory(""), mAward(""),
      mUnits_Token(gNullStr), mDifficulty(0), mPassive_Msg_Channel(gNullStr),
      mPassive_Msg_Priority(-1), mRequires_Unison_Ability(false), mPlayerCount_Min(-1),
      mPlayerCount_Max(-1), mNumSongs(-1), mProgressStep(0), mGamerpic_Reward(-1),
      mAvatarAsset_Reward(-1), mShowBestAfterEarn(true), mHideProgress(false), unk5c(i),
      mLastID(-1), mEarnedNoFail(true), mLeaderboard(false), isSecondaryGoal(false),
      mGiveToAll(false) {
    Configure(a);
}

Accomplishment::~Accomplishment() {}

bool Accomplishment::ShowBestAfterEarn() const { return mShowBestAfterEarn; }

bool Accomplishment::CanBeLaunched() const {
    static Symbol acc_calibrate("acc_calibrate");
    static Symbol acc_charactercreate("acc_charactercreate");

    if (mAccomplishment_Name == acc_calibrate) {
        return true;
    } else
        return mAccomplishment_Name == acc_charactercreate;
}

void Accomplishment::UpdateIncrementalEntryName(UILabel *, Symbol) {
    MILO_ASSERT(false, 0x4c);
}

Symbol Accomplishment::GetCategory() const { return mCategory; }

bool Accomplishment::HasGamerpicReward() const { return mGamerpic_Reward != -1; }

bool Accomplishment::HasAvatarAssetReward() const { return mAvatarAsset_Reward != -1; }

Symbol Accomplishment::GetAward() const { return mAward; }

bool Accomplishment::HasAward() const { return !(mAward == ""); }

bool Accomplishment::IsSecondaryGoal() const { return isSecondaryGoal; }

bool Accomplishment::IsDynamic() const { return !m_vDynamic_Prereq_Songs.empty(); }

char const *Accomplishment::GetIconArt() const {
    return MakeString(
        "ui/accomplishments/accomplishment_art/%s_keep.png", mAccomplishment_Name.Str()
    );
}

void Accomplishment::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x34);
    mAccomplishment_Name = i_pConfig->Sym(0);

    int launchabledifficulty = 0;
    static Symbol launchable_difficulty("launchable_difficulty");
    if (i_pConfig->FindData(launchable_difficulty, launchabledifficulty, false)) {
        mDifficulty = launchabledifficulty;
    }

    static Symbol launchable_playercount_min("launchable_playercount_min");
    i_pConfig->FindData(launchable_playercount_min, mPlayerCount_Min, false);

    static Symbol launchable_playercount_max("launchable_playercount_max");
    i_pConfig->FindData(launchable_playercount_max, mPlayerCount_Max, false);

    static Symbol launchable_requires_unison_ability("launchable_requires_unison_ability");
    i_pConfig->FindData(
        launchable_requires_unison_ability, mRequires_Unison_Ability, false
    );

    static Symbol secret_prereqs("secret_prereqs");
    DataArray *preReqs = i_pConfig->FindArray(secret_prereqs, false);
    if (preReqs) {
        for (int i = 1; i < preReqs->Size(); i++) {
            Symbol s = preReqs->Sym(i);
            m_vSecret_Prereqs.push_back(s);
        }
    }

    static Symbol dynamic_prereqs("dynamic_prereqs");
    DataArray *dynPreReqs = i_pConfig->FindArray(dynamic_prereqs, false);
    if (dynPreReqs) {
        static Symbol num_songs("num_songs");
        dynPreReqs->FindData(num_songs, mNumSongs, false);

        static Symbol songs("songs");
        DataArray *songArray = dynPreReqs->FindArray(songs);
        if (songArray) {
            for (int i = 0; i < songArray->Size(); i++) {
                Symbol s = songArray->Sym(i);
                m_vDynamic_Prereq_Songs.push_back(s);
            }
        }
        if (m_vDynamic_Prereq_Songs.size() < mNumSongs) {
            MILO_NOTIFY(
                "There are less songs in the dynamic prereq song list than the num_songs provided: %s",
                mAccomplishment_Name.Str()
            );
        }
    }

    static Symbol passive_msg_channel("passive_msg_channel");
    i_pConfig->FindData(passive_msg_channel, mPassive_Msg_Channel, false);

    static Symbol passive_msg_priority("passive_msg_priority");
    i_pConfig->FindData(passive_msg_priority, mPassive_Msg_Priority, false);

    if (mPassive_Msg_Channel != gNullStr) {
        if (mPassive_Msg_Priority < 1) {
            MILO_NOTIFY(
                "Passive Message Priority for goal %s is less than the minimum: %i!",
                mAccomplishment_Name.Str(),
                1
            );
            mPassive_Msg_Priority = 1;
        } else if (1000 < mPassive_Msg_Priority) {
            MILO_NOTIFY(
                "Passive Message Priority for goal %s is more than the minimum: %i!",
                mAccomplishment_Name.Str(),
                1000
            );
            mPassive_Msg_Priority = 1000;
        }
    }

    static Symbol progress_step("progress_step");
    i_pConfig->FindData(progress_step, mProgressStep, false);

    static Symbol show_best_after_earn("show_best_after_earn");
    i_pConfig->FindData(show_best_after_earn, mShowBestAfterEarn, false);

    static Symbol hide_progress("hide_progress");
    i_pConfig->FindData(hide_progress, mHideProgress, false);

    static Symbol can_be_earned_with_no_fail("can_be_earned_with_no_fail");
    i_pConfig->FindData(can_be_earned_with_no_fail, mEarnedNoFail, false);

    static Symbol leaderboard("leaderboard");
    i_pConfig->FindData(leaderboard, mLeaderboard, false);

    static Symbol give_to_all("give_to_all");
    i_pConfig->FindData(give_to_all, mGiveToAll, false);

    static Symbol xlast_id("xlast_id");
    i_pConfig->FindData(xlast_id, mLastID, false);

    static Symbol gamerpic_reward("gamerpic_reward");
    DataArray *gamerPicArray = i_pConfig->FindArray(gamerpic_reward, false);
    if (gamerPicArray) {
        Symbol s = gamerPicArray->Sym(1);
        if (s == nullptr) {
            mGamerpic_Reward = gamerPicArray->Int(0);
        } else {
            MILO_NOTIFY(
                "Invalid gamerpic_reward for %s (data type 0x%x)", mAccomplishment_Name, s
            );
        }
    }

    static Symbol avatarasset_reward("avatarasset_reward");
    DataArray *avatarArray = i_pConfig->FindArray(avatarasset_reward, false);
    if (avatarArray) {
        Symbol s = avatarArray->Sym(1);
        if (s == nullptr) {
            mAvatarAsset_Reward = avatarArray->Int(0);
        } else {
            MILO_NOTIFY(
                "Invalid avatarasset_reward for %s (data type 0x%x)",
                mAccomplishment_Name,
                s
            );
        }
    }

    static Symbol accomplishment_type("accomplishment_type");
    i_pConfig->FindData(accomplishment_type, mAccomplishment_Type);

    static Symbol category("category");
    i_pConfig->FindData(category, mCategory);

    static Symbol award("award");
    i_pConfig->FindData(award, mAward, false);

    static Symbol is_secondary_goal("is_secondary_goal");
    i_pConfig->FindData(is_secondary_goal, isSecondaryGoal, false);

    static Symbol units_token("units_token");
    i_pConfig->FindData(units_token, mUnits_Token, false);
}
