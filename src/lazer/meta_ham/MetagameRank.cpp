#include "meta_ham/MetagameRank.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/HamPlayerData.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "math/Rand.h"
#include <algorithm>
#include <cstdio>

namespace {
    DataArray *gRanksArray;
    DataArray *gRepeatableTasks;
    DataArray *gOneTimeTasks;

    // size 0x14
    struct DeferredAward {
        String unk0;
        Symbol unk8;
        Symbol unkc;
        Symbol unk10;
    };

    // size 0x20
    struct Unlockable {
        int unk0;
        Symbol unk4;
        Symbol unk8;
        Symbol unkc;
        Symbol unk10;
        std::vector<Symbol> unk14;
    };
    std::vector<Unlockable> gUnlockables;
    std::vector<Unlockable *> gTiers;
    std::list<DeferredAward> gDeferredAwardQueue;
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

bool MetagameRank::HasNewRank() const {
    if (!mAtMaxRank) {
        return mPctToNextRank == 1;
    } else {
        return unkc9;
    }
}

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
    int sum;
    if (mDeferredPoints.size() != 0) {
        sum = 0;
        FOREACH (it, mDeferredPoints) {
            sum += it->unk0;
        }
    } else {
        sum = 0;
    }
    SaveSymbolID(fs, combined_xp_disp);
    fs << sum;
    const_cast<MetagameRank *>(this)->unkca = false;
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
            mDeferredPoints.push_back(pt);
        }
    }
    ComputeRankNumber(true);
    unkca = false;
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
        gDeferredAwardQueue.pop_front();
        DataArrayPtr ptr(Symbol(award.unk0.c_str()), award.unk8, award.unkc, award.unk10);
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
    xp_force_award_one_time = 0;
    xp_force_award_all = 0;
    xp_force_one_rank_up = 0;
    DataRegisterFunc("xp_have_deferred_award", HaveDeferredAward);
    DataRegisterFunc("xp_deferred_award", HandleDeferredAward);
    DataArray *rankCfg = SystemConfig("rank");
    DataArray *unlockArr = rankCfg->FindArray("unlockables");
    if (unlockArr) {
        int newSize = unlockArr->Size() - 1;
        gUnlockables.resize(newSize);
        for (int i = 0; i < newSize; i++) {
            DataArray *curUnlockArray = unlockArr->Array(i + 1);
            Unlockable &cur = gUnlockables[i];
            cur.unk0 = i + 1;
            cur.unk4 = curUnlockArray->Sym(0);
            cur.unk8 = curUnlockArray->FindSym("name");
            cur.unkc = curUnlockArray->FindSym("desc");
            cur.unk10 = curUnlockArray->FindSym("image");
            DataArray *unlocksToPopulate = curUnlockArray->FindArray("unlock");
            cur.unk14.resize(unlocksToPopulate->Size() - 1);
            for (int j = 1; j < unlocksToPopulate->Size(); j++) {
                cur.unk14[j - 1] = unlocksToPopulate->Sym(j);
                // TheAccomplishmentManager AddAssetAward
            }
        }
    }
    DataArray *tierArr = rankCfg->FindArray("tiers");
    if (tierArr) {
        int newSize = tierArr->Size() - 1;
        gTiers.resize(newSize);
        for (int i = 0; i < newSize; i++) {
            DataArray *innerTierArr = tierArr->FindArray(i + 1);
            int innerSize = innerTierArr->Size();
            gTiers.reserve(innerSize);
            for (int j = 0; j < innerSize; j++) {
                bool b3 = false;
                Symbol s128 = innerTierArr->Sym(j);
                // there's more
            }
        }
    }
    DataArray *taskArr = rankCfg->FindArray("tasks");
    gRepeatableTasks = taskArr->FindArray("repeatable");
    gOneTimeTasks = rankCfg->FindArray("one_time");
}

void MetagameRank::Clear() {
    unkca = false;
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
        for (int i = 1; i < gOneTimeTasks->Size(); i++) {
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
        unkca = true;
        DataArrayPtr ptr(pt.unk4, pt.unk0);
        return ptr;
    }
}

extern PropertyEventProvider *TheHamProvider;

bool compare_deferred_points(const DeferredPoints &a, const DeferredPoints &b);

void MetagameRank::UpdateScore(
    int songID,
    const HamPlayerData *playerData,
    const SongStatusMgr *statusMgr,
    int stars,
    int unk
) {
    // Check if in party mode - early return (NOT static - constructed each call)
    if (TheHamProvider->Property(Symbol("is_in_party_mode"), true)->Int(0)) {
        return;
    }

    if (TheHamProvider->Property(Symbol("is_in_infinite_party_mode"), true)->Int(0)) {
        return;
    }

    // Static symbols for various awards - initialized with bit flags pattern
    // Group 1 - first 32 bits
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
    static Symbol random_bonus_occurs_1pct_of_the_time("random_bonus_occurs_1pct_of_the_time");
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
    // Group 2 - second 32 bits
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

    // Handle first time play bonus
    if (unk38) {
        unk38 = false;
        AwardPointsForTask(play_first_time);
    }

    // Debug force award variables
    static DataNode &xp_force_award_small = DataVariable("xp_force_award_small");
    static DataNode &xp_force_award_medium = DataVariable("xp_force_award_medium");
    static DataNode &xp_force_award_large = DataVariable("xp_force_award_large");
    static DataNode &xp_force_award_one_time = DataVariable("xp_force_award_one_time");
    static DataNode &xp_force_award_all = DataVariable("xp_force_award_all");
    static DataNode &xp_force_one_rank_up = DataVariable("xp_force_one_rank_up");

    // Handle force award small
    if (xp_force_award_small.Int(0)) {
        static Symbol smallTasks[] = {
            new_song_completed_on_beginner,
            new_song_completed_on_easy,
            new_song_completed_on_medium,
            new_song_completed_on_hard,
            maccoy_birthday,
            dare_birthday,
            glitch_birthday,
            completed_song_moderate,
            completed_song_tough,
            completed_song_legit,
            completed_song_hardcore,
            completed_song_off_the_hook,
            random_bonus_occurs_1pct_of_the_time,
            challenge_attempt,
            nail_fatality,
            perfect_performance_no_misses,
            emilia_birthday,
            completed_song_with_1_star,
            completed_song_with_2_stars,
            completed_song_with_3_stars,
            completed_song_with_4_stars,
            completed_song_with_5_stars,
            completed_song_on_beginner,
            completed_song_on_easy,
            completed_song_on_medium,
            completed_song_on_hard,
            golden_performance,
            completed_song_warmup,
            completed_song_simple,
            bodie_birthday,
            taye_birthday,
            lilt_birthday,
            angel_birthday,
            aubrey_birthday,
            mo_birthday,
            dlc_bonus,
            challenge_met,
            fitness_bonus,
            playlist_bonus,
            oblio_birthday,
            kerith_birthday
        };
        int idx = RandomInt(0, 0x29);
        Symbol task = smallTasks[idx];
        TheDebug << MakeString("XP Forcing Small Task: %s\n", task);
        AwardPointsForTask(task);
    }

    // Handle force award medium
    if (xp_force_award_medium.Int(0)) {
        static Symbol mediumTasks[] = {
            completed_song_with_2_stars,
            completed_song_on_hard,
            completed_song_off_the_hook,
            completed_song_with_3_stars,
            completed_song_with_4_stars,
            new_era_completed_campaign_70s,
            new_era_completed_campaign_80s,
            new_era_completed_campaign_90s,
            new_era_completed_campaign_00s,
            new_era_completed_campaign_10s,
            campaign_completed_on_easy_3
        };
        int idx = RandomInt(0, 0xb);
        Symbol task = mediumTasks[idx];
        AwardPointsForTask(task);
        TheDebug << MakeString("XP Forcing Medium Task: %s\n", task);
    }

    // Handle force award large
    if (xp_force_award_large.Int(0)) {
        static Symbol largeTasks[] = {
            completed_song_with_5_stars,
            campaign_completed_on_medium,
            campaign_completed_on_hard,
            five_star_a_characters_songlist
        };
        int idx = RandomInt(0, 0x4);
        Symbol task = largeTasks[idx];
        TheDebug << MakeString("XP Forcing Large Task: %s\n", task);
        AwardPointsForTask(task);
    }

    // Handle force award one time
    if (xp_force_award_one_time.Int(0)) {
        static Symbol oneTimeTasks[] = {
            jaryn_birthday,
            new_era_completed_campaign_70s,
            new_era_completed_campaign_80s,
            new_era_completed_campaign_90s,
            new_era_completed_campaign_00s,
            new_era_completed_campaign_10s,
            campaign_completed_on_easy_3,
            campaign_completed_on_medium,
            campaign_completed_on_hard,
            five_star_a_characters_songlist
        };
        int idx;
        for (int i = 0; i < 10; i++) {
            Symbol task = oneTimeTasks[i];
            int taskIdx = -1;
            if (GetOneTimeTask(task, nullptr, &taskIdx)) {
                MILO_ASSERT(taskIdx >= 0 && taskIdx < 0x40, 0x36F);
                if (unk39[taskIdx]) {
                    continue;
                }
            }
            AwardPointsForTask(task);
            TheDebug << MakeString("XP Forcing One Time Task: %s\n", task);
        }
    }

    // Skip normal scoring if any force award is active
    if (xp_force_award_small.Int(0) || xp_force_award_medium.Int(0) ||
        xp_force_award_large.Int(0) || xp_force_award_one_time.Int(0) ||
        xp_force_one_rank_up.Int(0)) {
        mDeferredPoints.sort(compare_deferred_points);
        return;
    }

    // Sort deferred points at the end
    mDeferredPoints.sort(compare_deferred_points);
}
