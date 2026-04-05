#include "meta_ham/MetagameRank.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "game/GamePanel.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/PoseFatalities.h"
#include "math/Utl.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/Campaign.h"
#include "meta_ham/CampaignEra.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/SongStatusMgr.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/System.h"
#include "math/Rand.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include <algorithm>
#include <climits>
#include <cstdio>

namespace {
    DataArray *gRanksArray;
    DataArray *gRepeatableTasks;
    DataArray *gOneTimeTasks;

    // size 0x20
    struct Unlockable {
        int unk0;
        Symbol unk4;
        Symbol unk8;
        Symbol unkc;
        Symbol unk10;
        std::vector<Symbol> unk14;
    };

    // size 0xc
    struct DeferredAward {
        String unk0; // 0x0 - user name?
        const Unlockable *unk8; // 0x8
    };

    std::vector<Unlockable> gUnlockables;
    std::vector<std::vector<Unlockable *> > gTiers;
    std::list<DeferredAward> gDeferredAwardQueue;

    void
    BuildUnlockablesList(const bool *bList, std::vector<const Unlockable *> &unlocks) {
        for (int i = 0; i < gTiers.size(); i++) {
            if (!unlocks.empty())
                break;
            for (int j = 0; j < gTiers[i].size(); j++) {
                Unlockable *cur = gTiers[i][j];
                if (!bList[cur->unk0]) {
                    unlocks.push_back(cur);
                }
            }
        }
        std::random_shuffle(unlocks.begin(), unlocks.end());
    }

}

MetagameRank::MetagameRank(HamProfile *p) : mProfile(p) {
    Clear();
    mSaveSizeMethod = SaveSize;
}

BEGIN_HANDLERS(MetagameRank)
    HANDLE_EXPR(get_score, mScore)
    HANDLE_EXPR(get_rank_number, mRankNumber)
    HANDLE_EXPR(get_rank_in_tier, GetRankInTier())
    HANDLE_EXPR(get_tier, GetTier())
    HANDLE_EXPR(get_xp_of_rank, GetXPOfRank(_msg->Int(2)))
    HANDLE_EXPR(has_new_rank, HasNewRank())
    HANDLE_EXPR(at_max_rank, mAtMaxRank)
    HANDLE_EXPR(get_percent_to_next_rank, mPctToNextRank)
    HANDLE_ACTION(award_points, AwardPointsForTask(_msg->Sym(2)))
    HANDLE_EXPR(have_deferred_points, mDeferredPoints.size() > 0)
    HANDLE(get_next_deferred_points, GetNextDeferredPoints)
END_HANDLERS

void MetagameRank::SaveFixed(FixedSizeSaveableStream &fs) const {
    fs << mScore;
    bool b1 = unk38;
    if (!b1) {
        static Symbol play_first_time_disp("play_first_time_disp");
        FOREACH (it, mDeferredPoints) {
            if (it->unk4 == play_first_time_disp) {
                b1 = true;
                break;
            }
        }
    }
    fs << b1;
    fs << mAtMaxRank;
    fs.Write(unk39, 0x40);
    fs.Write(unk79, 0x40);
    static Symbol combined_xp_disp("combined_xp_disp");
    Symbol s = combined_xp_disp; // lmao
    int sum;
    if (mDeferredPoints.size() != 0) {
        sum = 0;
        FOREACH (it, mDeferredPoints) {
            sum += it->unk0;
        }
    } else {
        sum = 0;
    }
    SaveSymbolID(fs, s);
    fs << sum;
    const_cast<MetagameRank *>(this)->mDirty = false;
}

void MetagameRank::LoadFixed(FixedSizeSaveableStream &fs, int i2) {
    fs >> mScore;
    if (i2 > 0x45) {
        fs >> unk38;
    }
    if (i2 > 0x4E) {
        fs >> mAtMaxRank;
    }
    fs.Read(unk39, 0x40);
    if (unk38) {
        int idx = -1;
        static Symbol play_first_time("play_first_time");
        GetOneTimeTask(play_first_time, nullptr, &idx);
        if (idx >= 0) {
            unk39[idx] = 0;
        }
    }
    fs.Read(unk79, 0x40);
    if (i2 > 0x3D) {
        if (i2 <= 0x5A) {
            int x;
            fs >> x;
        }
    }
    if (i2 > 0x5A) {
        DeferredPoints pt;
        LoadSymbolFromID(fs, pt.unk4);
        fs >> pt.unk0;
        if (pt.unk0 > 0) {
            mDeferredPoints.push_front(pt);
        }
    }
    ComputeRankNumber(true);
    mDirty = false;
}

bool MetagameRank::HasNewRank() const {
    if (!mAtMaxRank) {
        return mPctToNextRank == 1;
    } else {
        return unkc9;
    }
}

int MetagameRank::SaveSize(int i1) {
    int i2 = 4;
    if (i1 > 0x45) {
        i2 = 5;
    }
    if (i1 > 0x4E) {
        i2++;
    }
    int ret = i2 + 0x80;
    if (i1 > 0x3D) {
        if (i1 > 0x5A) {
            goto end;
        }
        ret += 4;
    }
    if (i1 <= 0x5A) {
        return ret;
    }
end:
    return ret + 8;
}

void MetagameRank::Preinit() {
    DataArray *rankCfg = SystemConfig("rank");
    gRanksArray = rankCfg->FindArray("ranks");
}

DataNode HaveDeferredAward(DataArray *) { return !gDeferredAwardQueue.empty(); }

DataNode HandleDeferredAward(DataArray *) {
    if (gDeferredAwardQueue.empty()) {
        return 0;
    } else {
        DeferredAward award = gDeferredAwardQueue.front();
        const Unlockable *unlockable = award.unk8;
        gDeferredAwardQueue.pop_front();
        DataArrayPtr ptr(
            Symbol(award.unk0.c_str()),
            unlockable->unk8,
            unlockable->unkc,
            unlockable->unk10
        );
        return ptr;
    }
}

void MetagameRank::Init() {
    static DataNode &xp_force_award_small = DataVariable("xp_force_award_small");
    static DataNode &xp_force_award_medium = DataVariable("xp_force_award_medium");
    static DataNode &xp_force_award_large = DataVariable("xp_force_award_large");
    static DataNode &xp_force_award_one_time = DataVariable("xp_force_award_one_time");
    static DataNode &xp_force_award_all = DataVariable("xp_force_award_all");
    static DataNode &xp_force_one_rank_up = DataVariable("xp_force_one_rank_up");
    xp_force_award_small = 0;
    xp_force_award_medium = 0;
    xp_force_award_large = 0;
    xp_force_award_large = 0;
    xp_force_award_all = 0;
    xp_force_one_rank_up = 0;
    DataRegisterFunc("xp_have_deferred_award", HaveDeferredAward);
    DataRegisterFunc("xp_deferred_award", HandleDeferredAward);
    int u11 = 0;
    DataArray *rankCfg = SystemConfig("rank");
    DataArray *unlockArr = rankCfg->FindArray("unlockables");
    if (unlockArr) {
        u11 = unlockArr->Size() - 1;
        gUnlockables.resize(u11);
        for (int i = 0; i < u11; i++) {
            DataArray *curUnlockArray = unlockArr->Array(i + 1);
            Unlockable &cur = gUnlockables[i];
            cur.unk0 = i;
            cur.unk4 = curUnlockArray->Sym(0);
            cur.unk8 = curUnlockArray->FindSym("name");
            cur.unkc = curUnlockArray->FindSym("desc");
            cur.unk10 = curUnlockArray->FindSym("image");
            DataArray *unlocksToPopulate = curUnlockArray->FindArray("unlock");
            cur.unk14.resize(unlocksToPopulate->Size() - 1);
            for (int j = 1; j < unlocksToPopulate->Size(); j++) {
                cur.unk14[j - 1] = unlocksToPopulate->Sym(j);
                TheAccomplishmentMgr->AddAssetAward(cur.unk14[j - 1], cur.unk4);
            }
        }
    }
    DataArray *tierArr = rankCfg->FindArray("tiers");
    if (tierArr) {
        int newSize = tierArr->Size() - 1;
        gTiers.resize(newSize);
        for (int i = 0; i < newSize; i++) {
            DataArray *innerTierArr = tierArr->Array(i + 1);
            int innerSize = innerTierArr->Size();
            gTiers[i].reserve(innerSize);
            for (int j = 0; j < innerSize; j++) {
                bool b3 = false;
                Symbol s128 = innerTierArr->Sym(j);
                for (int k = 0; k < u11; k++) {
                    Unlockable &cur = gUnlockables[k];
                    if (cur.unk4 == s128) {
                        gTiers[i].push_back(&cur);
                        b3 = true;
                        break;
                    }
                }
                if (!b3) {
                    MILO_FAIL("Unlock named %s not found in unlock list", s128.Str());
                }
            }
        }
    }
    DataArray *taskArr = rankCfg->FindArray("tasks");
    gRepeatableTasks = taskArr->FindArray("repeatable");
    gOneTimeTasks = taskArr->FindArray("one_time");
}

void MetagameRank::Clear() {
    mDirty = false;
    mScore = 0;
    unk38 = true;
    memset(unk39, 0, 0x40);
    memset(unk79, 0, 0x40);
    mDeferredPoints.clear();
    mRankNumber = 0;
    mAtMaxRank = false;
    unkc9 = false;
    mPctToNextRank = 0;
    ComputeRankNumber(true);
}

Symbol MetagameRank::GetRankTitle() const {
    char buf[32];
    sprintf(buf, "rank_%d", mRankNumber);
    return buf;
}

bool MetagameRank::GetOneTimeTask(Symbol s, DataArray **aptr, int *iptr) {
    if (aptr || iptr) {
        int aSize = gOneTimeTasks->Size();
        for (int i = 1; i < aSize; i++) {
            DataNode &n = gOneTimeTasks->Node(i);
            if (n.Type() == kDataArray) {
                if (n.Array()->Sym(0) == s) {
                    if (aptr) {
                        *aptr = n.Array();
                    }
                    if (iptr) {
                        *iptr = i - 1;
                    }
                    return true;
                }
            }
        }
        if (aptr) {
            *aptr = nullptr;
        }
        if (iptr) {
            *iptr = -1;
        }
    }
    return false;
}

int MetagameRank::GetRankInTier() const {
    if (mAtMaxRank) {
        return 0;
    }
    int i3 = mRankNumber;
    for (int i = 0; i < gTiers.size(); i++) {
        int i2 = gTiers[i].size();
        if (i2 >= i3) {
            return i3;
        }
        i3 -= i2;
    }
    return i3;
}

int MetagameRank::GetTier() const {
    if (mRankNumber == 0) {
        return 0;
    }
    int i3 = mRankNumber;
    unsigned int numTiers = gTiers.size();
    for (int i = 0; i < numTiers; i++) {
        for (int j = 0; j < gTiers[i].size(); j++) {
            if (--i3 <= 0) {
                return i + 1;
            }
        }
    }
    return numTiers;
}

int MetagameRank::GetXPOfRank(int i) const {
    int ret = 0;
    if (i > 0 && i < gRanksArray->Size() - 1) {
        ret = gRanksArray->Array(i + 1)->Int(0);
        if (i >= 1) {
            ret -= gRanksArray->Array(i)->Int(0);
        }
    }
    return ret;
}

DataNode MetagameRank::GetNextDeferredPoints(DataArray *a) {
    if (mDeferredPoints.empty()) {
        static Symbol xp_previous_points_msg("xp_previous_points_msg");
        return xp_previous_points_msg;
    } else {
        DeferredPoints pt = mDeferredPoints.front();
        mDeferredPoints.pop_front();
        mScore += pt.unk0;
        ComputeRankNumber(false);
        mDirty = true;
        DataArrayPtr ptr(pt.unk4, pt.unk0);
        return ptr;
    }
}

bool compare_deferred_points(DeferredPoints a, DeferredPoints b) {
    static Symbol play_first_time_disp("play_first_time_disp");
    static Symbol combined_xp_disp("combined_xp_disp");
    static Symbol double_xp_weekend_disp("double_xp_weekend_disp");
    static Symbol new_song_completed_on_beginner_disp(
        "new_song_completed_on_beginner_disp"
    );
    static Symbol new_song_completed_on_easy_disp("new_song_completed_on_easy_disp");
    static Symbol new_song_completed_on_medium_disp("new_song_completed_on_medium_disp");
    static Symbol new_song_completed_on_hard_disp("new_song_completed_on_hard_disp");
    static Symbol completed_song_with_1_star_disp("completed_song_with_1_star_disp");
    static Symbol completed_song_with_2_stars_disp("completed_song_with_2_stars_disp");
    static Symbol completed_song_with_3_stars_disp("completed_song_with_3_stars_disp");
    static Symbol completed_song_with_4_stars_disp("completed_song_with_4_stars_disp");
    static Symbol completed_song_with_5_stars_disp("completed_song_with_5_stars_disp");
    static Symbol completed_song_on_easy_disp("completed_song_on_easy_disp");
    static Symbol completed_song_on_medium_disp("completed_song_on_medium_disp");
    static Symbol completed_song_on_hard_disp("completed_song_on_hard_disp");
    static Symbol completed_song_warmup_disp("completed_song_warmup_disp");
    static Symbol completed_song_simple_disp("completed_song_simple_disp");
    static Symbol completed_song_moderate_disp("completed_song_moderate_disp");
    static Symbol completed_song_tough_disp("completed_song_tough_disp");
    static Symbol completed_song_legit_disp("completed_song_legit_disp");
    static Symbol completed_song_hardcore_disp("completed_song_hardcore_disp");
    static Symbol completed_song_off_the_hook_disp("completed_song_off_the_hook_disp");
    static Symbol nail_fatality_disp("nail_fatality_disp");
    static Symbol golden_performance_disp("golden_performance_disp");
    static Symbol perfect_performance_no_misses_disp("perfect_performance_no_misses_disp");
    static Symbol fitness_bonus_disp("fitness_bonus_disp");
    static Symbol playlist_bonus_disp("playlist_bonus_disp");
    static Symbol dlc_bonus_disp("dlc_bonus_disp");
    static Symbol challenge_met_disp("challenge_met_disp");
    static Symbol challenge_attempt_disp("challenge_attempt_disp");
    static Symbol emilia_birthday_disp("emilia_birthday_disp");
    static Symbol bodie_birthday_disp("bodie_birthday_disp");
    static Symbol taye_birthday_disp("taye_birthday_disp");
    static Symbol lilt_birthday_disp("lilt_birthday_disp");
    static Symbol angel_birthday_disp("angel_birthday_disp");
    static Symbol aubrey_birthday_disp("aubrey_birthday_disp");
    static Symbol mo_birthday_disp("mo_birthday_disp");
    static Symbol glitch_birthday_disp("glitch_birthday_disp");
    static Symbol dare_birthday_disp("dare_birthday_disp");
    static Symbol maccoy_birthday_disp("maccoy_birthday_disp");
    static Symbol oblio_birthday_disp("oblio_birthday_disp");
    static Symbol kerith_birthday_disp("kerith_birthday_disp");
    static Symbol jaryn_birthday_disp("jaryn_birthday_disp");
    static Symbol rasa_birthday_disp("rasa_birthday_disp");
    static Symbol lima_birthday_disp("lima_birthday_disp");
    static Symbol robota_birthday_disp("robota_birthday_disp");
    static Symbol robotb_birthday_disp("robotb_birthday_disp");
    static Symbol tan_birthday_disp("tan_birthday_disp");
    static Symbol tanrobot_birthday_disp("tanrobot_birthday_disp");
    static Symbol ninjaman_birthday_disp("ninjaman_birthday_disp");
    static Symbol ninjawoman_birthday_disp("ninjawoman_birthday_disp");
    static Symbol iconmanblue_birthday_disp("iconmanblue_birthday_disp");
    static Symbol iconmanpink_birthday_disp("iconmanpink_birthday_disp");
    static Symbol random_bonus_occurs_1pct_of_the_time_disp(
        "random_bonus_occurs_1pct_of_the_time_disp"
    );
    static Symbol new_era_completed_campaign_70s_disp(
        "new_era_completed_campaign_70s_disp"
    );
    static Symbol new_era_completed_campaign_80s_disp(
        "new_era_completed_campaign_80s_disp"
    );
    static Symbol new_era_completed_campaign_90s_disp(
        "new_era_completed_campaign_90s_disp"
    );
    static Symbol new_era_completed_campaign_00s_disp(
        "new_era_completed_campaign_00s_disp"
    );
    static Symbol new_era_completed_campaign_10s_disp(
        "new_era_completed_campaign_10s_disp"
    );
    static Symbol campaign_completed_on_easy_3_disp("campaign_completed_on_easy_3_disp");
    static Symbol campaign_completed_on_medium_disp("campaign_completed_on_medium_disp");
    static Symbol campaign_completed_on_hard_disp("campaign_completed_on_hard_disp");
    static Symbol five_star_a_characters_songlist_disp(
        "five_star_a_characters_songlist_disp"
    );
    static Symbol sDispArray[] = { play_first_time_disp,
                                   combined_xp_disp,
                                   double_xp_weekend_disp,
                                   new_song_completed_on_beginner_disp,
                                   new_song_completed_on_easy_disp,
                                   new_song_completed_on_medium_disp,
                                   new_song_completed_on_hard_disp,
                                   completed_song_with_1_star_disp,
                                   completed_song_with_2_stars_disp,
                                   completed_song_with_3_stars_disp,
                                   completed_song_with_4_stars_disp,
                                   completed_song_with_5_stars_disp,
                                   completed_song_on_easy_disp,
                                   completed_song_on_medium_disp,
                                   completed_song_on_hard_disp,
                                   completed_song_warmup_disp,
                                   completed_song_simple_disp,
                                   completed_song_moderate_disp,
                                   completed_song_tough_disp,
                                   completed_song_legit_disp,
                                   completed_song_hardcore_disp,
                                   completed_song_off_the_hook_disp,
                                   nail_fatality_disp,
                                   golden_performance_disp,
                                   perfect_performance_no_misses_disp,
                                   fitness_bonus_disp,
                                   playlist_bonus_disp,
                                   dlc_bonus_disp,
                                   challenge_met_disp,
                                   challenge_attempt_disp,
                                   emilia_birthday_disp,
                                   bodie_birthday_disp,
                                   taye_birthday_disp,
                                   lilt_birthday_disp,
                                   angel_birthday_disp,
                                   aubrey_birthday_disp,
                                   mo_birthday_disp,
                                   glitch_birthday_disp,
                                   dare_birthday_disp,
                                   maccoy_birthday_disp,
                                   oblio_birthday_disp,
                                   kerith_birthday_disp,
                                   jaryn_birthday_disp,
                                   rasa_birthday_disp,
                                   lima_birthday_disp,
                                   robota_birthday_disp,
                                   robotb_birthday_disp,
                                   tan_birthday_disp,
                                   tanrobot_birthday_disp,
                                   ninjaman_birthday_disp,
                                   ninjawoman_birthday_disp,
                                   iconmanblue_birthday_disp,
                                   iconmanpink_birthday_disp,
                                   random_bonus_occurs_1pct_of_the_time_disp,
                                   new_era_completed_campaign_70s_disp,
                                   new_era_completed_campaign_80s_disp,
                                   new_era_completed_campaign_90s_disp,
                                   new_era_completed_campaign_00s_disp,
                                   new_era_completed_campaign_10s_disp,
                                   campaign_completed_on_easy_3_disp,
                                   campaign_completed_on_medium_disp,
                                   campaign_completed_on_hard_disp,
                                   five_star_a_characters_songlist_disp };
    static std::map<Symbol, int> award_sort_indices;
    static bool indicesInitted = false;
    if (!indicesInitted) {
        for (int i = 0; i < DIM(sDispArray); i++) {
            award_sort_indices.insert(std::make_pair(sDispArray[i], i));
        }
        indicesInitted = true;
    }
    auto ait = award_sort_indices.find(a.unk4);
    auto bit = award_sort_indices.find(b.unk4);
    int aIndex = INT_MAX;
    int bIndex = INT_MAX;
    if (ait != award_sort_indices.end()) {
        aIndex = ait->second;
    } else {
        MILO_LOG(
            "WARNING: XP Task for %s not in sort order. It should be added.\n",
            a.unk4.Str()
        );
    }
    if (bit != award_sort_indices.end()) {
        bIndex = bit->second;
    } else {
        MILO_LOG(
            "WARNING: XP Task for %s not in sort order. It should be added.\n",
            b.unk4.Str()
        );
    }
    return aIndex < bIndex;
}

// whoever solves this will gain us 0x2194 bytes for lazer:
// https://decomp.me/scratch/m3yNS
void MetagameRank::UpdateScore(
    int songID,
    const HamPlayerData *playerData,
    const SongStatusMgr *statusMgr,
    int i4,
    int stars
) {
    if (!TheHamProvider->Property("is_in_party_mode")->Int()
        && !TheHamProvider->Property("is_in_infinite_party_mode")->Int()) {
        static Symbol double_xp_weekend("double_xp_weekend");
        static Symbol completed_song_with_1_star("completed_song_with_1_star");
        static Symbol completed_song_with_2_stars("completed_song_with_2_stars");
        static Symbol completed_song_with_3_stars("completed_song_with_3_stars");
        static Symbol completed_song_with_4_stars("completed_song_with_4_stars");
        static Symbol completed_song_with_5_stars("completed_song_with_5_stars");
        static Symbol completed_song_on_beginner("completed_song_on_beginner");
        static Symbol completed_song_on_easy("completed_song_on_easy");
        static Symbol completed_song_on_medium("completed_song_on_medium");
        static Symbol completed_song_on_hard("completed_song_on_hard");
        static Symbol golden_performance("golden_performance");
        static Symbol completed_song_warmup("completed_song_warmup");
        static Symbol completed_song_simple("completed_song_simple");
        static Symbol completed_song_moderate("completed_song_moderate");
        static Symbol completed_song_tough("completed_song_tough");
        static Symbol completed_song_legit("completed_song_legit");
        static Symbol completed_song_hardcore("completed_song_hardcore");
        static Symbol completed_song_off_the_hook("completed_song_off_the_hook");
        static Symbol random_bonus_occurs_1pct_of_the_time(
            "random_bonus_occurs_1pct_of_the_time"
        );
        static Symbol new_song_completed_on_beginner("new_song_completed_on_beginner");
        static Symbol new_song_completed_on_easy("new_song_completed_on_easy");
        static Symbol new_song_completed_on_medium("new_song_completed_on_medium");
        static Symbol new_song_completed_on_hard("new_song_completed_on_hard");
        static Symbol fitness_bonus("fitness_bonus");
        static Symbol playlist_bonus("playlist_bonus");
        static Symbol dlc_bonus("dlc_bonus");
        static Symbol challenge_met("challenge_met");
        static Symbol challenge_attempt("challenge_attempt");
        static Symbol nail_fatality("nail_fatality");
        static Symbol perfect_performance_no_misses("perfect_performance_no_misses");
        static Symbol emilia_birthday("emilia_birthday");
        static Symbol bodie_birthday("bodie_birthday");
        static Symbol taye_birthday("taye_birthday");
        static Symbol lilt_birthday("lilt_birthday");
        static Symbol angel_birthday("angel_birthday");
        static Symbol aubrey_birthday("aubrey_birthday");
        static Symbol mo_birthday("mo_birthday");
        static Symbol glitch_birthday("glitch_birthday");
        static Symbol dare_birthday("dare_birthday");
        static Symbol maccoy_birthday("maccoy_birthday");
        static Symbol oblio_birthday("oblio_birthday");
        static Symbol kerith_birthday("kerith_birthday");
        static Symbol jaryn_birthday("jaryn_birthday");
        static Symbol rasa_birthday("rasa_birthday");
        static Symbol lima_birthday("lima_birthday");
        static Symbol robota_birthday("robota_birthday");
        static Symbol robotb_birthday("robotb_birthday");
        static Symbol tan_birthday("tan_birthday");
        static Symbol tanrobot_birthday("tanrobot_birthday");
        static Symbol ninjaman_birthday("ninjaman_birthday");
        static Symbol ninjawoman_birthday("ninjawoman_birthday");
        static Symbol iconmanblue_birthday("iconmanblue_birthday");
        static Symbol iconmanpink_birthday("iconmanpink_birthday");
        static Symbol play_first_time("play_first_time");
        static Symbol new_era_completed_campaign_70s("new_era_completed_campaign_70s");
        static Symbol new_era_completed_campaign_80s("new_era_completed_campaign_80s");
        static Symbol new_era_completed_campaign_90s("new_era_completed_campaign_90s");
        static Symbol new_era_completed_campaign_00s("new_era_completed_campaign_00s");
        static Symbol new_era_completed_campaign_10s("new_era_completed_campaign_10s");
        static Symbol campaign_completed_on_easy_3("campaign_completed_on_easy_3");
        static Symbol campaign_completed_on_medium("campaign_completed_on_medium");
        static Symbol campaign_completed_on_hard("campaign_completed_on_hard");
        static Symbol five_star_a_characters_songlist("five_star_a_characters_songlist");

        if (unk38) {
            unk38 = false;
            AwardPointsForTask(play_first_time);
        }

        static DataNode &xp_force_award_small = DataVariable("xp_force_award_small");
        static DataNode &xp_force_award_medium = DataVariable("xp_force_award_medium");
        static DataNode &xp_force_award_large = DataVariable("xp_force_award_large");
        static DataNode &xp_force_award_one_time =
            DataVariable("xp_force_award_one_time");
        static DataNode &xp_force_award_all = DataVariable("xp_force_award_all");
        static DataNode &xp_force_one_rank_up = DataVariable("xp_force_one_rank_up");

        if (xp_force_award_small.Int()) {
            static Symbol sSmallTasks[] = { new_song_completed_on_beginner,
                                            new_song_completed_on_easy,
                                            new_song_completed_on_medium,
                                            new_song_completed_on_hard,
                                            completed_song_with_1_star,
                                            completed_song_on_medium,
                                            completed_song_moderate,
                                            completed_song_tough,
                                            completed_song_legit,
                                            completed_song_hardcore,
                                            nail_fatality,
                                            golden_performance,
                                            perfect_performance_no_misses,
                                            fitness_bonus,
                                            playlist_bonus,
                                            dlc_bonus,
                                            emilia_birthday,
                                            bodie_birthday,
                                            taye_birthday,
                                            lilt_birthday,
                                            angel_birthday,
                                            aubrey_birthday,
                                            mo_birthday,
                                            glitch_birthday,
                                            dare_birthday,
                                            maccoy_birthday,
                                            oblio_birthday,
                                            kerith_birthday,
                                            jaryn_birthday,
                                            rasa_birthday,
                                            lima_birthday,
                                            robota_birthday,
                                            robotb_birthday,
                                            tan_birthday,
                                            tanrobot_birthday,
                                            ninjaman_birthday,
                                            ninjawoman_birthday,
                                            iconmanblue_birthday,
                                            iconmanpink_birthday,
                                            random_bonus_occurs_1pct_of_the_time,
                                            play_first_time };
            Symbol task = sSmallTasks[RandomInt(0, DIM(sSmallTasks))];
            MILO_LOG("XP Forcing Small Task: %s\n", task.Str());
            AwardPointsForTask(task);
        }
        if (xp_force_award_medium.Int()) {
            static Symbol sMediumTasks[] = {
                completed_song_with_2_stars,    completed_song_on_hard,
                completed_song_off_the_hook,    completed_song_with_3_stars,
                completed_song_with_4_stars,    new_era_completed_campaign_70s,
                new_era_completed_campaign_80s, new_era_completed_campaign_90s,
                new_era_completed_campaign_00s, new_era_completed_campaign_10s,
                campaign_completed_on_easy_3,
            };
            Symbol task = sMediumTasks[RandomInt(0, DIM(sMediumTasks))];
            AwardPointsForTask(task);
            MILO_LOG("XP Forcing Medium Task: %s\n", task.Str());
        }
        if (xp_force_award_large.Int()) {
            static Symbol sLargeTasks[] = { completed_song_with_5_stars,
                                            campaign_completed_on_medium,
                                            campaign_completed_on_hard,
                                            five_star_a_characters_songlist };
            Symbol task = sLargeTasks[RandomInt(0, DIM(sLargeTasks))];
            MILO_LOG("XP Forcing Large Task: %s\n", task.Str());
            AwardPointsForTask(task);
        }
        if (xp_force_award_one_time.Int()) {
            static Symbol sOneTimeTasks[] = { play_first_time,
                                              new_era_completed_campaign_70s,
                                              new_era_completed_campaign_80s,
                                              new_era_completed_campaign_90s,
                                              new_era_completed_campaign_00s,
                                              new_era_completed_campaign_10s,
                                              campaign_completed_on_easy_3,
                                              campaign_completed_on_medium,
                                              campaign_completed_on_hard,
                                              five_star_a_characters_songlist };

            bool b25 = false;
            for (int i = 0; i < DIM(sOneTimeTasks); i++) {
                Symbol curTask = sOneTimeTasks[i];
                int task_index = -1;
                if (GetOneTimeTask(curTask, nullptr, &task_index)) {
                    MILO_ASSERT(task_index >= 0 && task_index < kMaxTasksOneTime, 0x36F);
                    if (!unk39[task_index]) {
                        MILO_LOG("XP Forcing One-Time Task: %s\n", curTask.Str());
                        AwardPointsForTask(curTask);
                        b25 = true;
                        break;
                    }
                }
            }
            if (!b25) {
                MILO_LOG(
                    "XP Forcing One-Time Task: ALL ONE-TIME TASKS HAVE BEEN COMPLETED\n"
                );
            }
        }

        if (xp_force_award_small.Int() || xp_force_award_medium.Int()
            || xp_force_award_large.Int() || xp_force_award_one_time.Int()) {
            // do nothing, return
        } else if (xp_force_one_rank_up.Int()) {
            MILO_LOG("XP Forcing One Rank Up\n");
            static Symbol played_1000_songs_disp("played_1000_songs_disp");
            AwardPoints(GetXPOfRank(mRankNumber), played_1000_songs_disp);
        } else if (xp_force_award_all.Int()) {
            MILO_LOG("XP Forcing Awarding All Ranks\n");
            float f37 = 1.0f;
            if (TheRockCentral.GetUnk8c()) {
                f37 = 0.5f;
            }
            static Symbol played_1000_songs_disp("played_1000_songs_disp");
            for (int i = mRankNumber; i < 65; i++) {
                AwardPoints((float)GetXPOfRank(i) * f37, played_1000_songs_disp);
            }
            xp_force_award_all = 0;
        } else {
            if (RandomInt(0, 100) == 42) {
                AwardPointsForTask(random_bonus_occurs_1pct_of_the_time);
            }
            if (TheRockCentral.GetUnk8c()) {
                AwardPointsForTask(double_xp_weekend);
            }
            if (stars >= 6) {
                AwardPointsForTask(completed_song_with_5_stars);
                AwardPointsForTask(golden_performance);
            }
            switch (stars) {
            case 1:
                AwardPointsForTask(completed_song_with_1_star);
                break;
            case 2:
                AwardPointsForTask(completed_song_with_2_stars);
                break;
            case 3:
                AwardPointsForTask(completed_song_with_3_stars);
                break;
            case 4:
                AwardPointsForTask(completed_song_with_4_stars);
                break;
            case 5:
                AwardPointsForTask(completed_song_with_5_stars);
                break;
            }
            switch (TheGameData->Player(playerData->Unk40())->GetDifficulty()) {
            case kDifficultyEasy:
                AwardPointsForTask(completed_song_on_easy);
                break;
            case kDifficultyMedium:
                AwardPointsForTask(completed_song_on_medium);
                break;
            case kDifficultyExpert:
                AwardPointsForTask(completed_song_on_hard);
                break;
            case kDifficultyBeginner:
                AwardPointsForTask(completed_song_on_beginner);
                break;
            }
            const HamSongMetadata *songMetadata = TheHamSongMgr.Data(songID);
            if (songMetadata) {
                switch (TheHamSongMgr.RankTier(songMetadata->Rank())) {
                case 0:
                    AwardPointsForTask(completed_song_warmup);
                    break;
                case 1:
                    AwardPointsForTask(completed_song_simple);
                    break;
                case 2:
                    AwardPointsForTask(completed_song_moderate);
                    break;
                case 3:
                    AwardPointsForTask(completed_song_tough);
                    break;
                case 4:
                    AwardPointsForTask(completed_song_legit);
                    break;
                case 5:
                    AwardPointsForTask(completed_song_hardcore);
                    break;
                case 6:
                    AwardPointsForTask(completed_song_off_the_hook);
                    break;
                }
                for (int i = 0; i < 2; i++) {
                    HamPlayerData *player_data = TheGameData->Player(i);
                    MILO_ASSERT(player_data, 0x3E0);
                    Hmx::Object *player_provider = player_data->Provider();
                    MILO_ASSERT(player_provider, 0x3E3);
                }
                int i17 = TheHamProvider->Property("stars_earned", false)->Int();
                if (i17 >= 5) {
                    Symbol hChar = songMetadata->Character();
                    SongStatusMgr *profileSongStatusMgr = mProfile->GetSongStatusMgr();
                    bool bref = false;
                    int i19 = profileSongStatusMgr->GetStars(songID, bref);
                    i19 = Clamp(0, 5, i19);
                    i17 = Clamp(0, 5, i17);
                    int sub = i17 - i19;
                    int i58 = 0;
                    int i48 = 0;
                    TheHamSongMgr.GetCharacterStars(mProfile, hChar, i58, i48);
                    i58 += sub;
                    if (i48 >= 0 && i58 >= i48) {
                        AwardPointsForTask(five_star_a_characters_songlist);
                    }
                }
                if (songMetadata->IsDownload()) {
                    AwardPointsForTask(dlc_bonus);
                }
                static Symbol is_in_campaign_mode("is_in_campaign_mode");
                if (TheHamProvider->Property(is_in_campaign_mode)->Int()) {
                    CampaignPerformer *performer =
                        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
                    if (performer) {
                        CampaignEra *era = TheCampaign->GetCampaignEra(performer->Era());
                        Symbol shortname = TheHamSongMgr.GetShortNameFromSongID(songID);
                        Symbol craze = era->GetDanceCrazeSong();
                        Symbol eraName = era->GetName();
                        static Symbol era01("era01");
                        static Symbol era02("era02");
                        static Symbol era03("era03");
                        static Symbol era04("era04");
                        static Symbol era05("era05");
                        static Symbol era_tan_battle("era_tan_battle");
                        if (shortname == craze) {
                            if (eraName == Symbol("era01")) {
                                AwardPointsForTask(new_era_completed_campaign_70s);
                            } else if (eraName == Symbol("era02")) {
                                AwardPointsForTask(new_era_completed_campaign_80s);
                            } else if (eraName == Symbol("era03")) {
                                AwardPointsForTask(new_era_completed_campaign_90s);
                            } else if (eraName == Symbol("era04")) {
                                AwardPointsForTask(new_era_completed_campaign_00s);
                            } else if (eraName == Symbol("era05")) {
                                AwardPointsForTask(new_era_completed_campaign_10s);
                            }
                        } else if (eraName == Symbol("era_tan_battle")) {
                            switch (
                                TheGameData->Player(playerData->Unk40())->GetDifficulty()
                            ) {
                            case kDifficultyEasy:
                                AwardPointsForTask(campaign_completed_on_easy_3);
                                break;
                            case kDifficultyMedium:
                                AwardPointsForTask(campaign_completed_on_medium);
                                break;
                            case kDifficultyExpert:
                                AwardPointsForTask(campaign_completed_on_hard);
                                break;
                            }
                        }
                    }
                }
            }
            Difficulty d1 = TheGameData->Player(playerData->Unk40())->GetDifficulty();
            if (!statusMgr->IsSongPlayed(songID)
                || IsHarderDifficulty(d1, statusMgr->GetDifficulty(songID))) {
                switch (d1) {
                case kDifficultyEasy:
                    AwardPointsForTask(new_song_completed_on_easy);
                    break;
                case kDifficultyMedium:
                    AwardPointsForTask(new_song_completed_on_medium);
                    break;
                case kDifficultyExpert:
                    AwardPointsForTask(new_song_completed_on_hard);
                    break;
                case kDifficultyBeginner:
                    AwardPointsForTask(new_song_completed_on_beginner);
                    break;
                }
            }
            if (mProfile && mProfile->InFitnessMode()) {
                AwardPointsForTask(fitness_bonus);
            }
            if (TheGameMode->InMode("playlist_perform")) {
                AwardPointsForTask(playlist_bonus);
            }
            DataNode handled = TheGamePanel->Handle(Message("num_rated_measures"), false);
            if (handled.Type() != kDataUnhandled) {
                int numPerfects =
                    playerData->Provider()->Property("num_perfect", false)->Int();
                int perfectsDanced = handled.Int();
                if (numPerfects == perfectsDanced && numPerfects > 0) {
                    AwardPointsForTask(perfect_performance_no_misses);
                }
            }
            Symbol birthdayChars[] = {
                "emilia",     "bodie",       "taye",       "lilt",     "angel",
                "aubrey",     "mo",          "glitch",     "dare",     "maccoy",
                "oblio",      "kerith",      "jaryn",      "rasa",     "lima",
                "robota",     "robotb",      "tan",        "tanrobot", "ninjaman",
                "ninjawoman", "iconmanblue", "iconmanpink"
            };
            DateTime dt;
            GetDateAndTime(dt);
            static Symbol birthday("birthday");
            DataArray *birthdayCfg = SystemConfig(birthday);
            for (int i = 0; i < DIM(birthdayChars); i++) {
                if (playerData->Char() == birthdayChars[i]) {
                    DataArray *a =
                        birthdayCfg->FindArray(birthday)->FindArray(birthdayChars[i]);
                    if (a->Node(1).Int() == dt.mMonth + 1
                        && a->Node(2).Int() == dt.mDay) {
                        char birthdayBuffer[256];
                        sprintf(birthdayBuffer, "%s_birthday", birthdayChars[i].Str());
                        AwardPointsForTask(birthdayBuffer);
                    }
                }
            }
            static Symbol challenge("challenge");
            static Symbol challenge_met_disp("challenge_met_disp");
            if (TheGameMode->InMode(challenge)) {
                std::vector<int> vec;
                if (TheChallenges->GetBeatenChallengeXPs(playerData, i4, vec)) {
                    if (vec.size() != 0) {
                        for (int i = 0; i < vec.size(); i++) {
                            AwardPoints(vec[i], challenge_met_disp);
                            if (mProfile) {
                                mProfile->IncrementChallengesMet();
                            }
                        }
                    } else {
                        AwardPointsForTask(challenge_attempt);
                    }
                }
            }
            if (!TheGameMode->InMode("strike_a_pose")) {
                if (TheHamDirector->GetPoseFatalities()
                    && TheHamDirector->GetPoseFatalities()->GotFullCombo(
                        playerData->Unk40()
                    )) {
                    AwardPointsForTask(nail_fatality);
                }
            }
            mDeferredPoints.sort(compare_deferred_points);
        }
    }
}

void MetagameRank::AwardPoints(int i, Symbol s) {
    if (TheRockCentral.GetUnk8c()) {
        i = i << 1;
    }
    DeferredPoints df;
    df.unk0 = i;
    df.unk4 = s;
    mDeferredPoints.push_back(df);
    mDirty = true;
}

void MetagameRank::AwardPointsForTask(Symbol task) {
    static Symbol score("score");
    static Symbol display("display");
    DataArray *taskArray = gRepeatableTasks->FindArray(task, false);
    if (!taskArray) {
        int task_index = -1;
        bool oneTimeTask = GetOneTimeTask(task, &taskArray, &task_index);
        if (!oneTimeTask) {
            MILO_FAIL("Task %s not found in metagame_rank.dta", task.Str());
        }

        MILO_ASSERT(task_index < kMaxTasksOneTime, 0x19b);
        if (!oneTimeTask) {
            return;
        }
        if (unk39[task_index] != 0) {
            return;
        }
        unk39[task_index] = 1;
    }

    int scoreNum = taskArray->FindInt(score);
    Symbol disp = taskArray->FindSym(display);
    if (0 <= scoreNum) {
        AwardPoints(scoreNum, disp);
    }
}

void MetagameRank::AwardForRankUp(int i1) {
    std::vector<const Unlockable *> unlocks;
    auto it = unlocks.rbegin();
    for (; i1 > 0; i1--) {
        if (unlocks.empty()) {
            BuildUnlockablesList(unk79, unlocks);
            it = unlocks.rbegin();
            if (unlocks.empty()) {
                return;
            }
        }
        const Unlockable *cur = *++it;
        DeferredAward award;
        if (mProfile && mProfile->GetHamUser()) {
            award.unk0 = mProfile->GetHamUser()->UserName();
        }
        award.unk8 = cur;
        unk79[cur->unk0] = true;
        char buffer[16];
        memcpy(buffer, "no_unlock_", 11);
        if (strncmp(cur->unk4.Str(), buffer, strlen(buffer))) {
            gDeferredAwardQueue.push_back(award);
            FOREACH (sit, cur->unk14) {
                mProfile->UnlockContent(*sit);
            }
        }
    }
}

int MetagameRank::ComputeRankNumber(bool b1) {
    if (unk38) {
        mRankNumber = 0;
        mPctToNextRank = 0;
        return 0;
    } else {
        if (mAtMaxRank) {
            mPctToNextRank = 0;
            mAtMaxRank = true;
            mRankNumber = gRanksArray->Size() - 1;
        } else {
            int i7 = 1;
            int i6 = 0;
            for (; i7 < gRanksArray->Size(); i7++) {
                DataArray *curArr = gRanksArray->Array(i7);
                int i5 = curArr->Int(0);
                if (mScore < i5) {
                    i7--;
                    if (i7 != -1) {
                        mPctToNextRank = (float)(mScore - i6) / (float)(i5 - i6);
                        goto next;
                    }
                    break;
                }
                i6 = i5;
            }
            mAtMaxRank = true;
            i7 = gRanksArray->Size() - 1;
            mPctToNextRank = 0;
        next:
            if (mRankNumber != i7) {
                if (mAtMaxRank) {
                    unkc9 = true;
                }
                if (!b1) {
                    AwardForRankUp(i7 - mRankNumber);
                }
                mRankNumber = i7;
            }
        }
        return mRankNumber;
    }
}
