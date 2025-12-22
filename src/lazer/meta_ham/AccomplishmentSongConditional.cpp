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

void AccomplishmentSongConditional::UpdateIncrementalEntryName(UILabel *label, Symbol s) {
    AppLabel *pAppLabel = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(pAppLabel, 0xa3);
    pAppLabel->SetSongName(s, -1, false);
}

bool AccomplishmentSongConditional::InqProgressValues(
    HamProfile *profile, int &i2, int &i3
) {
    i3 = GetTotalNumSongs();
    i2 = 0;
    if (profile) {
        i2 = GetNumCompletedSongs(profile);
    }
    return true;
}

bool AccomplishmentSongConditional::IsSymbolEntryFulfilled(
    HamProfile *hp, Symbol s
) const {
    if (!hp)
        return false;
    else {
        SongStatusMgr *pSongStatusMgr = hp->GetSongStatusMgr();
        MILO_ASSERT(pSongStatusMgr, 0xc0);
        return CheckConditionsForSong(pSongStatusMgr, s);
    }
}

bool AccomplishmentSongConditional::CheckStarsCondition(
    SongStatusMgr *statusMgr, Symbol s, AccomplishmentCondition const &ac
) const {
    bool b;
    int songID = TheHamSongMgr.GetSongIDFromShortName(s);
    int bestStars = statusMgr->GetBestStars(songID, b, ac.mDifficulty);
    return (bestStars >= ac.unk4);
}

bool AccomplishmentSongConditional::CheckScoreCondition(
    SongStatusMgr *statusMgr, Symbol s, AccomplishmentCondition const &ac
) const {
    bool b;
    int songID = TheHamSongMgr.GetSongIDFromShortName(s);
    int bestScore = statusMgr->GetBestScore(songID, b, ac.mDifficulty);
    return (bestScore >= ac.unk4);
}

bool AccomplishmentSongConditional::CheckPracticePercentageCondition(
    SongStatusMgr *statusMgr, Symbol s, AccomplishmentCondition const &ac
) const {
    int songID = TheHamSongMgr.GetSongIDFromShortName(s);
    int pracScore = statusMgr->GetPracticeScore(songID);
    return (pracScore >= ac.unk4);
}

bool AccomplishmentSongConditional::CheckNoFlashcardsCondition(
    SongStatusMgr *statusMgr, Symbol s
) const {
    bool b;
    int songID = TheHamSongMgr.GetSongIDFromShortName(s);
    for (int i = 0; i < 4; i++) {
        int starsForDiff = statusMgr->GetStarsForDifficulty(songID, (Difficulty)i, b);
        if (b)
            return true;
    }
    return false;
}

bool AccomplishmentSongConditional::CheckConditionsForSong(
    SongStatusMgr *mgr, Symbol s
) const {
    static Symbol stars("stars");
    static Symbol score("score");
    static Symbol practice_percentage("practice_percentage");
    static Symbol played("played");
    FOREACH (it, m_lConditions) {
        const AccomplishmentCondition &curCond = *it;
        Symbol curSym = curCond.unk0;
        MetaPerformer *pMetaPerformer = MetaPerformer::Current();
        MILO_ASSERT(pMetaPerformer, 0x60);
        if (curCond.mNoFlashcards) {
            if (!CheckNoFlashcardsCondition(mgr, s)) {
                return false;
            }
        } else {
            if (curCond.mMode != gNullStr && TheGameMode->Mode() != curCond.mMode) {
                return false;
            }
        }
        if (curSym == stars) {
            if (CheckStarsCondition(mgr, s, curCond)) {
                return true;
            }
        } else if (curSym == score) {
            if (CheckScoreCondition(mgr, s, curCond)) {
                return true;
            }
        } else if (curSym == played) {
            if (mgr->IsSongPlayed(TheHamSongMgr.GetSongIDFromShortName(s))) {
                return true;
            }
        } else if (curSym == practice_percentage) {
            if (CheckPracticePercentageCondition(mgr, s, curCond)) {
                return true;
            }
        } else {
            MILO_NOTIFY("Condition is not currently supported: %s ", curSym);
            return false;
        }
    }
    return false;
}
