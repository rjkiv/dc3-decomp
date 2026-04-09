#include "lazer/meta_ham/Accomplishment.h"
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

static const unsigned int sMinPriority = 1;
static const unsigned int sMaxPriority = 1000;

Accomplishment::Accomplishment(DataArray *cfg, int idx)
    : mName(""), mAccomplishmentType(), mCategory(""), mAward(""), mUnitsToken(gNullStr),
      mDifficulty(kDifficultyEasy), mPassiveMsgChannel(gNullStr), mPassiveMsgPriority(-1),
      mRequiresUnisonAbility(false), mPlayerCountMin(-1), mPlayerCountMax(-1),
      mNumSongs(-1), mProgressStep(0), mGamerpicReward(-1), mAvatarAssetReward(-1),
      mShowBestAfterEarn(true), mHideProgress(false), mIndex(idx), mContextID(-1),
      mEarnedNoFail(true), mLeaderboard(false), mIsSecondaryGoal(false),
      mGiveToAll(false) {
    Configure(cfg);
}

Accomplishment::~Accomplishment() {}

bool Accomplishment::ShowBestAfterEarn() const { return mShowBestAfterEarn; }

Difficulty Accomplishment::GetRequiredDifficulty() const { return mDifficulty; }

bool Accomplishment::CanBeLaunched() const {
    static Symbol acc_calibrate("acc_calibrate");
    static Symbol acc_charactercreate("acc_charactercreate");

    if (mName == acc_calibrate) {
        return true;
    } else
        return mName == acc_charactercreate;
}

const std::vector<Symbol> &Accomplishment::GetDynamicPrereqsSongs() const {
    return mDynamicPrereqsSongs;
}

Symbol Accomplishment::GetName() const { return mName; }
Symbol Accomplishment::GetCategory() const { return mCategory; }
bool Accomplishment::HasGamerpicReward() const { return mGamerpicReward != -1; }
bool Accomplishment::HasAvatarAssetReward() const { return mAvatarAssetReward != -1; }
Symbol Accomplishment::GetAward() const { return mAward; }
bool Accomplishment::HasAward() const { return mAward != ""; }
bool Accomplishment::IsSecondaryGoal() const { return mIsSecondaryGoal; }
bool Accomplishment::IsDynamic() const { return !mDynamicPrereqsSongs.empty(); }
int Accomplishment::GetDynamicPrereqsNumSongs() const { return mNumSongs; }
int Accomplishment::GetAvatarAssetReward() const { return mAvatarAssetReward; }
int Accomplishment::GetContextID() const { return mContextID; }
int Accomplishment::GetGamerpicReward() const { return mGamerpicReward; }

char const *Accomplishment::GetIconArt() const {
    return MakeString("ui/accomplishments/accomplishment_art/%s_keep.png", mName.Str());
}

void Accomplishment::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x34);
    mName = i_pConfig->Sym(0);

    static Symbol launchable_difficulty("launchable_difficulty");
    int launchabledifficulty = 0;
    if (i_pConfig->FindData(launchable_difficulty, launchabledifficulty, false)) {
        mDifficulty = (Difficulty)launchabledifficulty;
    }
    static Symbol launchable_playercount_min("launchable_playercount_min");
    i_pConfig->FindData(launchable_playercount_min, mPlayerCountMin, false);
    static Symbol launchable_playercount_max("launchable_playercount_max");
    i_pConfig->FindData(launchable_playercount_max, mPlayerCountMax, false);
    static Symbol launchable_requires_unison_ability("launchable_requires_unison_ability");
    i_pConfig->FindData(launchable_requires_unison_ability, mRequiresUnisonAbility, false);
    static Symbol secret_prereqs("secret_prereqs");
    DataArray *preReqs = i_pConfig->FindArray(secret_prereqs, false);
    if (preReqs) {
        for (int i = 1; i < preReqs->Size(); i++) {
            Symbol s = preReqs->Sym(i);
            mSecretPrereqs.push_back(s);
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
            for (int i = 1; i < songArray->Size(); i++) {
                Symbol s = songArray->Sym(i);
                mDynamicPrereqsSongs.push_back(s);
            }
        }
        if (mDynamicPrereqsSongs.size() < mNumSongs) {
            MILO_NOTIFY(
                "There are less songs in the dynamic prereq song list than the num_songs provided: %s",
                mName.Str()
            );
            mNumSongs = -1;
        }
    }
    static Symbol passive_msg_channel("passive_msg_channel");
    i_pConfig->FindData(passive_msg_channel, mPassiveMsgChannel, false);
    static Symbol passive_msg_priority("passive_msg_priority");
    i_pConfig->FindData(passive_msg_priority, mPassiveMsgPriority, false);

    if (mPassiveMsgChannel != gNullStr) {
        if (mPassiveMsgPriority < 1) {
            MILO_NOTIFY(
                "Passive Message Priority for goal %s is less than the minimum: %i!",
                mName.Str(),
                sMinPriority
            );
            mPassiveMsgPriority = 1;
        } else if (mPassiveMsgPriority > 1000) {
            MILO_NOTIFY(
                "Passive Message Priority for goal %s is more than the minimum: %i!",
                mName.Str(),
                sMaxPriority
            );
            mPassiveMsgPriority = 1000;
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
    i_pConfig->FindData(xlast_id, mContextID, false);
    static Symbol gamerpic_reward("gamerpic_reward");
    DataArray *gamerPicArray = i_pConfig->FindArray(gamerpic_reward, false);
    if (gamerPicArray) {
        DataNode &n = gamerPicArray->Node(1);
        if (n.Type() == kDataInt) {
            mGamerpicReward = n.Int();
        } else {
            MILO_NOTIFY(
                "Invalid gamerpic_reward for %s (data type 0x%x)", mName, n.Type()
            );
        }
    }
    static Symbol avatarasset_reward("avatarasset_reward");
    DataArray *avatarArray = i_pConfig->FindArray(avatarasset_reward, false);
    if (avatarArray) {
        DataNode &n = avatarArray->Node(1);
        if (n.Type() == kDataInt) {
            mAvatarAssetReward = n.Int();
        } else {
            MILO_NOTIFY(
                "Invalid avatarasset_reward for %s (data type 0x%x)", mName, n.Type()
            );
        }
    }
    static Symbol accomplishment_type("accomplishment_type");
    int type;
    i_pConfig->FindData(accomplishment_type, type);
    mAccomplishmentType = (AccomplishmentType)type;
    static Symbol category("category");
    i_pConfig->FindData(category, mCategory);
    static Symbol award("award");
    i_pConfig->FindData(award, mAward, false);
    static Symbol is_secondary_goal("is_secondary_goal");
    i_pConfig->FindData(is_secondary_goal, mIsSecondaryGoal, false);
    static Symbol units_token("units_token");
    i_pConfig->FindData(units_token, mUnitsToken, false);
}
