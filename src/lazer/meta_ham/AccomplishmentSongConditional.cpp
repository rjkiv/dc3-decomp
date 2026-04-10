#include "meta_ham/AccomplishmentSongConditional.h"
#include "AccomplishmentConditional.h"
#include "HamProfile.h"
#include "HamSongMgr.h"
#include "game/GameMode.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

AccomplishmentSongConditional::AccomplishmentSongConditional(DataArray *d, int i)
    : AccomplishmentConditional(d, i) {}

AccomplishmentSongConditional::~AccomplishmentSongConditional() {}

void AccomplishmentSongConditional::UpdateIncrementalEntryName(
    UILabel *label, Symbol shortname
) {
    AppLabel *pAppLabel = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(pAppLabel, 0xa3);
    pAppLabel->SetSongName(shortname, -1, false);
}

bool AccomplishmentSongConditional::InqProgressValues(
    HamProfile *profile, int &completedSongs, int &totalSongs
) {
    totalSongs = GetTotalNumSongs();
    completedSongs = 0;
    if (profile) {
        completedSongs = GetNumCompletedSongs(profile);
    }
    return true;
}

bool AccomplishmentSongConditional::IsSymbolEntryFulfilled(
    HamProfile *profile, Symbol shortname
) const {
    if (!profile)
        return false;
    else {
        SongStatusMgr *pSongStatusMgr = profile->GetSongStatusMgr();
        MILO_ASSERT(pSongStatusMgr, 0xc0);
        return CheckConditionsForSong(pSongStatusMgr, shortname);
    }
}

bool AccomplishmentSongConditional::CheckStarsCondition(
    SongStatusMgr *statusMgr, Symbol shortname, AccomplishmentCondition const &ac
) const {
    bool b;
    int songID = TheHamSongMgr.GetSongIDFromShortName(shortname);
    int bestStars = statusMgr->GetBestStars(songID, b, ac.mDifficulty);
    return (bestStars >= ac.mValue);
}

bool AccomplishmentSongConditional::CheckScoreCondition(
    SongStatusMgr *statusMgr, Symbol shortname, AccomplishmentCondition const &ac
) const {
    bool b;
    int songID = TheHamSongMgr.GetSongIDFromShortName(shortname);
    int bestScore = statusMgr->GetBestScore(songID, b, ac.mDifficulty);
    return (bestScore >= ac.mValue);
}

bool AccomplishmentSongConditional::CheckPracticePercentageCondition(
    SongStatusMgr *statusMgr, Symbol shortname, AccomplishmentCondition const &ac
) const {
    int songID = TheHamSongMgr.GetSongIDFromShortName(shortname);
    int pracScore = statusMgr->GetPracticeScore(songID);
    return (pracScore >= ac.mValue);
}

bool AccomplishmentSongConditional::CheckNoFlashcardsCondition(
    SongStatusMgr *statusMgr, Symbol shortname
) const {
    bool b;
    int songID = TheHamSongMgr.GetSongIDFromShortName(shortname);
    for (int i = 0; i < kNumDifficulties; i++) {
        int starsForDiff = statusMgr->GetStarsForDifficulty(songID, (Difficulty)i, b);
        if (b)
            return true;
    }
    return false;
}

bool AccomplishmentSongConditional::CheckConditionsForSong(
    SongStatusMgr *mgr, Symbol shortname
) const {
    static Symbol stars("stars");
    static Symbol score("score");
    static Symbol practice_percentage("practice_percentage");
    static Symbol played("played");

    FOREACH (it, m_lConditions) {
        const AccomplishmentCondition &curCond = *it;
        Symbol curSym = curCond.mCondition;
        MetaPerformer *pMetaPerformer = MetaPerformer::Current();
        MILO_ASSERT(pMetaPerformer, 0x60);
        if (curCond.mNoFlashcards) {
            if (!CheckNoFlashcardsCondition(mgr, shortname)) {
                return false;
            }
        } else if (curCond.mMode != gNullStr && TheGameMode->Mode() != curCond.mMode) {
            return false;
        }
        if (curSym == stars) {
            if (CheckStarsCondition(mgr, shortname, curCond)) {
                return true;
            }
        } else if (curSym == score) {
            if (CheckScoreCondition(mgr, shortname, curCond)) {
                return true;
            }
        } else if (curSym == played) {
            if (mgr->IsSongPlayed(TheHamSongMgr.GetSongIDFromShortName(shortname))) {
                return true;
            }
        } else if (curSym == practice_percentage) {
            if (CheckPracticePercentageCondition(mgr, shortname, curCond)) {
                return true;
            }
        } else {
            MILO_NOTIFY("Condition is not currently supported: %s ", curSym);
            return false;
        }
    }
    return false;
}
