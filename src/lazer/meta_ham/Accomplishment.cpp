#include "lazer/meta_ham/Accomplishment.h"
#include "hamobj/Difficulty.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

static const unsigned int sMinPriority = 1;
static const unsigned int sMaxPriority = 1000;

Accomplishment::Accomplishment(DataArray *cfg, int idx)
    : m_symName(""), m_eType(), m_symCategory(""), m_symAward(""), m_symUnits(gNullStr),
      m_eLaunchableDifficulty(kDifficultyEasy), m_symPassiveMsgChannel(gNullStr),
      m_iPassiveMsgPriority(-1), m_bLaunchableRequiresUnisonAbility(false),
      m_iLaunchableMinPlayers(-1), m_iLaunchableMaxPlayers(-1),
      m_iDynamicPrereqsNumSongs(-1), m_iProgressStep(0), m_iGamerpicReward(-1),
      m_iAvatarAssetReward(-1), m_bShowBestAfterEarn(true), m_bHideProgress(false),
      m_iIndex(idx), m_iContextID(-1), m_bCanBeEarnedWithNoFail(true),
      m_bTrackedInLeaderboard(false), m_bIsSecondaryGoal(false), m_bGiveToAll(false) {
    Configure(cfg);
}

Accomplishment::~Accomplishment() {}

bool Accomplishment::ShowBestAfterEarn() const { return m_bShowBestAfterEarn; }

Difficulty Accomplishment::GetRequiredDifficulty() const {
    return m_eLaunchableDifficulty;
}

bool Accomplishment::CanBeLaunched() const {
    static Symbol acc_calibrate("acc_calibrate");
    static Symbol acc_charactercreate("acc_charactercreate");

    if (m_symName == acc_calibrate) {
        return true;
    } else
        return m_symName == acc_charactercreate;
}

const std::vector<Symbol> &Accomplishment::GetDynamicPrereqsSongs() const {
    return m_vDynamicPrereqsSongs;
}

Symbol Accomplishment::GetName() const { return m_symName; }
Symbol Accomplishment::GetCategory() const { return m_symCategory; }
bool Accomplishment::HasGamerpicReward() const { return m_iGamerpicReward != -1; }
bool Accomplishment::HasAvatarAssetReward() const { return m_iAvatarAssetReward != -1; }
Symbol Accomplishment::GetAward() const { return m_symAward; }
bool Accomplishment::HasAward() const { return m_symAward != ""; }
bool Accomplishment::IsSecondaryGoal() const { return m_bIsSecondaryGoal; }
bool Accomplishment::IsDynamic() const { return !m_vDynamicPrereqsSongs.empty(); }
int Accomplishment::GetDynamicPrereqsNumSongs() const {
    return m_iDynamicPrereqsNumSongs;
}
int Accomplishment::GetAvatarAssetReward() const { return m_iAvatarAssetReward; }
int Accomplishment::GetContextID() const { return m_iContextID; }
int Accomplishment::GetGamerpicReward() const { return m_iGamerpicReward; }

char const *Accomplishment::GetIconArt() const {
    return MakeString(
        "ui/accomplishments/accomplishment_art/%s_keep.png", m_symName.Str()
    );
}

void Accomplishment::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x34);
    m_symName = i_pConfig->Sym(0);

    static Symbol launchable_difficulty("launchable_difficulty");
    int launchabledifficulty = 0;
    if (i_pConfig->FindData(launchable_difficulty, launchabledifficulty, false)) {
        m_eLaunchableDifficulty = (Difficulty)launchabledifficulty;
    }
    static Symbol launchable_playercount_min("launchable_playercount_min");
    i_pConfig->FindData(launchable_playercount_min, m_iLaunchableMinPlayers, false);
    static Symbol launchable_playercount_max("launchable_playercount_max");
    i_pConfig->FindData(launchable_playercount_max, m_iLaunchableMaxPlayers, false);
    static Symbol launchable_requires_unison_ability("launchable_requires_unison_ability");
    i_pConfig->FindData(
        launchable_requires_unison_ability, m_bLaunchableRequiresUnisonAbility, false
    );
    static Symbol secret_prereqs("secret_prereqs");
    DataArray *preReqs = i_pConfig->FindArray(secret_prereqs, false);
    if (preReqs) {
        for (int i = 1; i < preReqs->Size(); i++) {
            Symbol s = preReqs->Sym(i);
            m_vSecretPrereqs.push_back(s);
        }
    }
    static Symbol dynamic_prereqs("dynamic_prereqs");
    DataArray *dynPreReqs = i_pConfig->FindArray(dynamic_prereqs, false);
    if (dynPreReqs) {
        static Symbol num_songs("num_songs");
        dynPreReqs->FindData(num_songs, m_iDynamicPrereqsNumSongs, false);
        static Symbol songs("songs");
        DataArray *songArray = dynPreReqs->FindArray(songs);
        if (songArray) {
            for (int i = 1; i < songArray->Size(); i++) {
                Symbol s = songArray->Sym(i);
                m_vDynamicPrereqsSongs.push_back(s);
            }
        }
        if (m_vDynamicPrereqsSongs.size() < m_iDynamicPrereqsNumSongs) {
            MILO_NOTIFY(
                "There are less songs in the dynamic prereq song list than the num_songs provided: %s",
                m_symName.Str()
            );
            m_iDynamicPrereqsNumSongs = -1;
        }
    }
    static Symbol passive_msg_channel("passive_msg_channel");
    i_pConfig->FindData(passive_msg_channel, m_symPassiveMsgChannel, false);
    static Symbol passive_msg_priority("passive_msg_priority");
    i_pConfig->FindData(passive_msg_priority, m_iPassiveMsgPriority, false);

    if (m_symPassiveMsgChannel != gNullStr) {
        if (m_iPassiveMsgPriority < 1) {
            MILO_NOTIFY(
                "Passive Message Priority for goal %s is less than the minimum: %i!",
                m_symName.Str(),
                sMinPriority
            );
            m_iPassiveMsgPriority = 1;
        } else if (m_iPassiveMsgPriority > 1000) {
            MILO_NOTIFY(
                "Passive Message Priority for goal %s is more than the minimum: %i!",
                m_symName.Str(),
                sMaxPriority
            );
            m_iPassiveMsgPriority = 1000;
        }
    }
    static Symbol progress_step("progress_step");
    i_pConfig->FindData(progress_step, m_iProgressStep, false);
    static Symbol show_best_after_earn("show_best_after_earn");
    i_pConfig->FindData(show_best_after_earn, m_bShowBestAfterEarn, false);
    static Symbol hide_progress("hide_progress");
    i_pConfig->FindData(hide_progress, m_bHideProgress, false);
    static Symbol can_be_earned_with_no_fail("can_be_earned_with_no_fail");
    i_pConfig->FindData(can_be_earned_with_no_fail, m_bCanBeEarnedWithNoFail, false);
    static Symbol leaderboard("leaderboard");
    i_pConfig->FindData(leaderboard, m_bTrackedInLeaderboard, false);
    static Symbol give_to_all("give_to_all");
    i_pConfig->FindData(give_to_all, m_bGiveToAll, false);
    static Symbol xlast_id("xlast_id");
    i_pConfig->FindData(xlast_id, m_iContextID, false);
    static Symbol gamerpic_reward("gamerpic_reward");
    DataArray *gamerPicArray = i_pConfig->FindArray(gamerpic_reward, false);
    if (gamerPicArray) {
        DataNode &n = gamerPicArray->Node(1);
        if (n.Type() == kDataInt) {
            m_iGamerpicReward = n.Int();
        } else {
            MILO_NOTIFY(
                "Invalid gamerpic_reward for %s (data type 0x%x)", m_symName, n.Type()
            );
        }
    }
    static Symbol avatarasset_reward("avatarasset_reward");
    DataArray *avatarArray = i_pConfig->FindArray(avatarasset_reward, false);
    if (avatarArray) {
        DataNode &n = avatarArray->Node(1);
        if (n.Type() == kDataInt) {
            m_iAvatarAssetReward = n.Int();
        } else {
            MILO_NOTIFY(
                "Invalid avatarasset_reward for %s (data type 0x%x)", m_symName, n.Type()
            );
        }
    }
    static Symbol accomplishment_type("accomplishment_type");
    int type;
    i_pConfig->FindData(accomplishment_type, type);
    m_eType = (AccomplishmentType)type;
    static Symbol category("category");
    i_pConfig->FindData(category, m_symCategory);
    static Symbol award("award");
    i_pConfig->FindData(award, m_symAward, false);
    static Symbol is_secondary_goal("is_secondary_goal");
    i_pConfig->FindData(is_secondary_goal, m_bIsSecondaryGoal, false);
    static Symbol units_token("units_token");
    i_pConfig->FindData(units_token, m_symUnits, false);
}
