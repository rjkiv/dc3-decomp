#include "meta_ham/MetagameStats.h"
#include "game/GameMode.h"
#include "game/GamePanel.h"
#include "hamobj/HamGameData.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetagameRank.h"
#include "meta_ham/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"

MetagameStats::MetagameStats() : mStatsCfg(SystemConfig("stats")) {
    Clear();
    mSaveSizeMethod = SaveSize;
}

BEGIN_HANDLERS(MetagameStats)
    HANDLE_ACTION(increment_count, IncrementCount((CountStatID)_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(
        tally_favorite,
        TallyFavorite((FavoriteStatID)_msg->Int(2), _msg->Int(3), _msg->Int(4))
    )
    HANDLE_EXPR(get_count, GetCount((CountStatID)_msg->Int(2)))
    HANDLE_EXPR(get_favorite, GetFavorite((FavoriteStatID)_msg->Int(2)))
    HANDLE_EXPR(
        get_favorite_count, GetFavoriteCount((FavoriteStatID)_msg->Int(2), _msg->Int(3))
    )
END_HANDLERS

void MetagameStats::Text(int, int data, UIListLabel *slot, UILabel *label) const {
    MILO_ASSERT(data >= 0 && data < NumData(), 0x69);
    DataArray *statArr = mStatsCfg->Array(data + 1);
    if (statArr->Int(0) == kStatsLayoutHeader) {
        if (slot->Matches("header")) {
            label->SetTextToken(statArr->Sym(1));
        }
    } else if (statArr->Int(0) != kStatsLayoutSpacer) {
        if (slot->Matches("stat") || slot->Matches("title")) {
            int id = statArr->Int(0); // count/fave stat id
            StatType statType = (StatType)statArr->Int(1); // stat type
            Symbol statSym = statArr->Sym(2); // the actual stat symbol
            DataArray *titleArr = statArr->FindArray("titles", false);
            Symbol scc(gNullStr);
            switch (statType) {
            case kStatType_Count: {
                int count = GetCount((CountStatID)id);
                if (slot->Matches("title")) {
                    Symbol title = SetTitleByThreshold(count, titleArr);
                    label->SetTextToken(title);
                } else {
                    const char *loc = LocalizeSeparatedInt(count, TheLocale);
                    label->SetTokenFmt(statSym, loc);
                }
                break;
            }
            case kStatType_Time: {
                int count = GetCount((CountStatID)id);
                if (slot->Matches("title")) {
                    Symbol title = SetTitleByThreshold(count, titleArr);
                    label->SetTextToken(title);
                } else {
                    char buf[32];
                    GetTimeString(count, buf);
                    label->SetTokenFmt(statSym, buf);
                }
                break;
            }
            case kStatType_FavoriteMode: {
                static Symbol stats_favorite_na("stats_favorite_na");
                static Symbol perform("perform");
                static Symbol practice("practice");
                static Symbol dancebattle("dancebattle");
                int fave = GetFavorite((FavoriteStatID)id);
                if (slot->Matches("title")) {
                    Symbol title = SetTitleForFavorite(fave, 0, nullptr);
                    label->SetTextToken(title);
                } else {
                    Symbol tag = stats_favorite_na;
                    if (fave == 0) {
                        tag = perform;
                    } else if (fave == 1) {
                        tag = practice;
                    } else if (fave == 2) {
                        tag = dancebattle;
                    }
                    label->SetTokenFmt(statSym, Localize(tag, nullptr, TheLocale));
                }
                break;
            }
            case kStatType_FavoriteSong: {
                static Symbol stats_favorite_na("stats_favorite_na");
                int fave = GetFavorite((FavoriteStatID)id);
                int faveCount = 0;
                const char *loc = Localize(stats_favorite_na, nullptr, TheLocale);
                if (fave != -1 && TheHamSongMgr.Data(fave)) {
                    loc = TheHamSongMgr.Data(fave)->Title();
                    faveCount = GetFavoriteCount((FavoriteStatID)id, fave);
                }
                if (slot->Matches("title")) {
                    Symbol title = SetTitleForFavorite(fave, faveCount, statArr);
                    label->SetTextToken(title);
                } else if (faveCount == 1) {
                    Symbol single = MakeString("%s_single", statSym.Str());
                    label->SetTokenFmt(single, loc);
                } else {
                    label->SetTokenFmt(
                        statSym, loc, LocalizeSeparatedInt(faveCount, TheLocale)
                    );
                }
                break;
            }
            case kStatType_FavoriteCharacter: {
                static Symbol stats_favorite_na("stats_favorite_na");
                int fave = GetFavorite((FavoriteStatID)id);
                Symbol s15 = stats_favorite_na;
                if (fave != -1) {
                    DataArray *characterArray =
                        SystemConfig()->FindArray("characters", false);
                    MILO_ASSERT(characterArray, 0xDF);
                    for (int i = 1; i < characterArray->Size(); i++) {
                        if (fave == i) {
                            DataArray *characterEntry = characterArray->Array(i);
                            MILO_ASSERT(characterEntry, 0xE5);
                            s15 = characterEntry->Sym(0);
                        }
                    }
                }
                if (slot->Matches("title")) {
                    Symbol title = SetTitleForFavorite(fave, 0, statArr);
                    label->SetTextToken(title);
                } else {
                    label->SetTokenFmt(statSym, s15);
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

RndMat *MetagameStats::Mat(int, int data, UIListMesh *slot) const {
    MILO_ASSERT(data >= 0 && data < NumData(), 0xFC);
    DataArray *arr = mStatsCfg->Array(data + 1);
    int i1 = arr->Int(0);
    if (i1 == -1 && slot->Matches("underline")) {
        return slot->DefaultMat();
    } else {
        return nullptr;
    }
}

void MetagameStats::SaveFixed(FixedSizeSaveableStream &fs) const {
    for (int i = 0; i < kNumCountStats; i++) {
        fs << mCountStats[i];
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        FixedSizeSaveable::SaveStd(fs, mFavoriteStats[i].mCounts, 1000, 8);
    }
    const_cast<MetagameStats *>(this)->mDirty = false;
}

void MetagameStats::LoadFixed(FixedSizeSaveableStream &fs, int) {
    for (int i = 0; i < kNumCountStats; i++) {
        fs >> mCountStats[i];
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        FixedSizeSaveable::LoadStd(fs, mFavoriteStats[i].mCounts, 1000, 8);
    }
    mDirty = false;
}

int MetagameStats::SaveSize(int) { return 0xDB38; }

void MetagameStats::Clear() {
    mDirty = false;
    for (int i = 0; i < kNumCountStats; i++) {
        mCountStats[i] = 0;
    }
    for (int i = 0; i < kNumFavoriteStats; i++) {
        mFavoriteStats[i].mCounts.clear();
    }
    unk140 = 0;
}

int MetagameStats::GetCount(CountStatID id) const {
    MILO_ASSERT(id >= 0 && id < kNumCountStats, 0x43);
    return mCountStats[id];
}

Symbol MetagameStats::SetTitleByThreshold(int i1, DataArray *a2) const {
    Symbol out(gNullStr);
    int i = 1;
    while (a2 && i < a2->Size()) {
        DataArray *curArr = a2->Array(i);
        if (i1 >= curArr->Int(0)) {
            out = curArr->Sym(1);
        }
        i++;
    }
    return out;
}

int MetagameStats::GetFavorite(FavoriteStatID id) const {
    MILO_ASSERT(id >= 0 && id < kNumFavoriteStats, 0x49);
    int ret = -1;
    int i4 = 0;
    FOREACH (it, mFavoriteStats[id].mCounts) {
        if (it->second > i4) {
            i4 = it->second;
            ret = it->first;
        }
    }
    return ret;
}

int MetagameStats::GetFavoriteCount(FavoriteStatID id, int key) const {
    MILO_ASSERT(id >= 0 && id < kNumFavoriteStats, 0x5E);
    MILO_ASSERT(mFavoriteStats[id].mCounts.find(key) != mFavoriteStats[id].mCounts.end(), 0x5F);
    return mFavoriteStats[id].mCounts.find(key)->second;
}

void MetagameStats::UpdatePartyStats(int x) {
    if (x > mCountStats[kCountStat_LongestPartyAttended]) {
        mCountStats[kCountStat_LongestPartyAttended] = x;
        mDirty = true;
    }
}

void MetagameStats::PhotoTaken() {
    mCountStats[kCountStat_PhotosTaken]++;
    mDirty = true;
    static Symbol perform("perform");
    static Symbol dance_battle("dance_battle");
    static Symbol gameplay_mode("gameplay_mode");
    if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
        mDirty = true;
        mCountStats[kCountStat_PhotosTakenPerform]++;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == dance_battle) {
        mDirty = true;
        mCountStats[kCountStat_PhotosTakenMultiplayer]++;
    }
}

void MetagameStats::TallyFavorite(FavoriteStatID id, int i2, int i3) {
    MILO_ASSERT(id >= 0 && id < kNumFavoriteStats, 0x3C);
    mFavoriteStats[id].mCounts[i2] += i3;
    mDirty = true;
}

void MetagameStats::WriteTimePlayed(HamProfile *profile, int i2) {
    mCountStats[kCountStat_TotalTimePlayed] += i2;
    mDirty = true;
    static Symbol perform("perform");
    static Symbol practice("practice");
    static Symbol dance_battle("dance_battle");
    static Symbol gameplay_mode("gameplay_mode");
    if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
        mDirty = true;
        mCountStats[kCountStat_TotalTimePerforming] += i2;
        mFavoriteStats[kFavoriteStat_FavoriteMode].mCounts[0] += i2;
        mDirty = true;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == practice) {
        mDirty = true;
        mCountStats[kCountStat_TotalTimeRehearsing] += i2;
        mFavoriteStats[kFavoriteStat_FavoriteMode].mCounts[1] += i2;
        mDirty = true;
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == dance_battle) {
        mDirty = true;
        mCountStats[kCountStat_TotalTimeMultiplayer] += i2;
        mFavoriteStats[kFavoriteStat_FavoriteMode].mCounts[2] += i2;
        mDirty = true;
    }
    unk140 += i2;
}

void MetagameStats::IncrementCount(CountStatID id, int cnt) {
    MILO_ASSERT(id >= 0 && id < kNumCountStats, 0x35);
    mCountStats[id] += cnt;
    mDirty = true;
}

Symbol MetagameStats::SetTitleForFavorite(int i1, int i2, DataArray *arr) const {
    Symbol ret(gNullStr);
    int i = 1;
    while (arr && i < arr->Size()) {
        DataArray *titleArray = arr->Array(i);
        if (i1 == titleArray->Int(0)) {
            if (titleArray->Size() == 2) {
                ret = titleArray->Sym(1);
            } else {
                if (i2 >= titleArray->Int(1)) {
                    ret = titleArray->Sym(2);
                }
            }
        }
        i++;
    }
    return ret;
}

bool MetagameStats::InqStatString(int statIndex, String &statStr) {
    MILO_ASSERT_RANGE(statIndex, 0, NumData(), 0x10B);
    DataArray *stat = mStatsCfg->Array(statIndex + 1);
    MILO_ASSERT(stat, 0x10D);
    int id = stat->Int(0);
    StatType statType = (StatType)stat->Int(1);
    Symbol statSym = stat->Sym(2);
    switch (statType) {
    case kStatType_Count: {
        statStr = MakeString(
            Localize(statSym, nullptr, TheLocale),
            LocalizeSeparatedInt(GetCount((CountStatID)id), TheLocale)
        );
        break;
    }
    case kStatType_Time: {
        char buf[32];
        int count = GetCount((CountStatID)id);
        GetTimeString(count, buf);
        statStr = MakeString(Localize(statSym, nullptr, TheLocale), buf);
        break;
    }
    case kStatType_FavoriteMode: {
        static Symbol stats_favorite_na("stats_favorite_na");
        static Symbol perform("perform");
        static Symbol practice("practice");
        static Symbol dancebattle("dancebattle");
        int fave = GetFavorite((FavoriteStatID)id);
        Symbol tag = stats_favorite_na;
        if (fave == 0) {
            tag = perform;
        } else if (fave == 1) {
            tag = practice;
        } else if (fave == 2) {
            tag = dancebattle;
        }
        statStr = MakeString(Localize(statSym, nullptr, TheLocale), tag);
        break;
    }
    case kStatType_FavoriteSong: {
        static Symbol stats_favorite_na("stats_favorite_na");
        int fave = GetFavorite((FavoriteStatID)id);
        int faveCount = 0;
        const char *localized = Localize(stats_favorite_na, nullptr, TheLocale);
        if (fave != -1 && TheHamSongMgr.Data(fave)) {
            localized = TheHamSongMgr.Data(fave)->Title();
            faveCount = GetFavoriteCount((FavoriteStatID)id, fave);
            if (faveCount == 1) {
                Symbol single = MakeString("%s_single", statSym.Str());
                statStr = MakeString(Localize(single, nullptr, TheLocale), localized);
                return true;
            }
        }
        statStr = MakeString(
            Localize(statSym, nullptr, TheLocale),
            localized,
            LocalizeSeparatedInt(faveCount, TheLocale)
        );
        break;
    }
    case kStatType_FavoriteCharacter: {
        static Symbol stats_favorite_na("stats_favorite_na");
        int fave = GetFavorite((FavoriteStatID)id);
        Symbol s9 = stats_favorite_na;
        if (fave != -1) {
            DataArray *characterArray = SystemConfig()->FindArray("characters", false);
            MILO_ASSERT(characterArray, 0x155);
            for (int i = 1; i < characterArray->Size(); i++) {
                if (fave == i) {
                    DataArray *characterEntry = characterArray->Array(i);
                    MILO_ASSERT(characterEntry, 0x15B);
                    s9 = MakeString("%s_title", characterEntry->Sym(0).Str());
                }
            }
        }
        statStr = MakeString(
            Localize(statSym, nullptr, TheLocale), Localize(s9, nullptr, TheLocale)
        );
        break;
    }
    default:
        break;
    }
    return true;
}

void MetagameStats::HandleGameplayEnded(
    HamProfile *profile, HamPlayerData *playerData, const EndGameResult &result
) {
    MILO_ASSERT(profile, 0x1A3);
    MILO_ASSERT(playerData, 0x1A4);
    int songID = TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong());
    IncFavoriteStat(kFavoriteStat_FavoriteSong, songID);
    int numStars = 0;
    if (result == 1 || result == 2) {
        static Symbol num_stars("num_stars");
        const DataNode *prop = TheGamePanel->Property(num_stars, false);
        if (prop) {
            numStars = prop->Float();
            if (numStars >= 5) {
                numStars = 5;
            }
        }
    }
    AddCount(kCountStat_StarsEarned, numStars);
    static Symbol perform("perform");
    static Symbol practice("practice");
    static Symbol dance_battle("dance_battle");
    static Symbol gameplay_mode("gameplay_mode");
    if (TheGameMode->Property(gameplay_mode)->Sym() == perform) {
        IncFavoriteStat(kFavoriteStat_FavoriteSongPerform, songID);
        AddCount(kCountStat_TimesPlayedPerform, 1);
        AddCount(kCountStat_StarsEarnedPerform, numStars);
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == practice) {
        IncFavoriteStat(kFavoriteStat_FavoriteSongPractice, songID);
        AddCount(kCountStat_TimesPlayedPractice, 1);
    }
    if (TheGameMode->Property(gameplay_mode)->Sym() == dance_battle) {
        IncFavoriteStat(kFavoriteStat_FavoriteSongMultiplayer, songID);
        AddCount(kCountStat_TimesPlayedMultiplayer, 1);
        AddCount(kCountStat_StarsEarnedMultiplayer, numStars);
    }
    if (profile->InFitnessMode()) {
        IncFavoriteStat(kFavoriteStat_FavoriteSongFitness, songID);
        AddCount(kCountStat_TotalFitnessSongs, 1);
    }

    Symbol playerChar = playerData->Char();
    int charIdx = -1;
    DataArray *charArray = SystemConfig()->FindArray("characters", false);
    if (charArray) {
        for (int i = 1; i < charArray->Size(); i++) {
            DataArray *pCharacterEntry = charArray->Array(i);
            MILO_ASSERT(pCharacterEntry, 0x1DA);
            if (playerChar == pCharacterEntry->Sym(0)) {
                charIdx = i;
            }
        }
        if (charIdx >= 1) {
            IncFavoriteStat(kFavoriteStat_FavoriteCharacter, charIdx);
        }
    }
    if (TheGameMode->IsGameplayModePerform()) {
        Hmx::Object *pPlayerProvider = playerData->Provider();
        MILO_ASSERT(pPlayerProvider, 0x1EA);
        static Symbol cumulative_score("cumulative_score");
        int ia0 = pPlayerProvider->Property(cumulative_score)->Int();
        MetaPerformer *mp = MetaPerformer::Current();
        int pad = playerData->PadNum();
        int time = mp->GetPlaylistElapsedTime();
        MILO_LOG(
            "MetagameStats::HandleGameplayEnded: Update InfinitePlaylist stats. player=%d, cumulativeScore=%d, cumulativeTime=%d\n",
            pad,
            ia0,
            time
        );
        UpdateInfinitePlaylistScore(ia0);
        UpdateInfinitePlaylistTime(time);
    }
}
