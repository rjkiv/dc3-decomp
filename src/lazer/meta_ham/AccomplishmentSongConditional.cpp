#include "meta_ham/AccomplishmentSongConditional.h"
#include "AccomplishmentConditional.h"
#include "HamProfile.h"
#include "HamSongMgr.h"
#include "hamobj/Difficulty.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

AccomplishmentSongConditional::AccomplishmentSongConditional(DataArray *d, int i)
    : AccomplishmentConditional(d, i) {}

AccomplishmentSongConditional::~AccomplishmentSongConditional() {}

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

bool AccomplishmentSongConditional::IsSymbolEntryFulfilled(
    HamProfile *hp, Symbol s
) const {
    if (!hp)
        return 0;
    else {
        SongStatusMgr *pSongStatusMgr = hp->GetSongStatusMgr();
        MILO_ASSERT(pSongStatusMgr, 0xc0);
    }
}

bool AccomplishmentSongConditional::CheckConditionsForSong(SongStatusMgr *, Symbol) const {
    static Symbol stars("stars");
    static Symbol score("score");
    static Symbol practice_percentage("practice_percentage");
    static Symbol played("played");
    FOREACH (it, m_lConditions) {
        MetaPerformer *pMetaPerformer = MetaPerformer::Current();
        MILO_ASSERT(pMetaPerformer, 0x60);
    }
    return false;
}
