#include "meta_ham/AccomplishmentDiscSongConditional.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/AccomplishmentSongConditional.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "os/Debug.h"

AccomplishmentDiscSongConditional::AccomplishmentDiscSongConditional(
    DataArray *cfg, int idx
)
    : AccomplishmentSongConditional(cfg, idx), mSongCount(0) {
    Configure(cfg);
}

AccomplishmentDiscSongConditional::~AccomplishmentDiscSongConditional() {}

bool AccomplishmentDiscSongConditional::IsFulfilled(HamProfile *profile) const {
    auto &songs = TheAccomplishmentMgr->GetDiscSongs();
    SongStatusMgr *mgr = profile->GetSongStatusMgr();
    int num = 0;
    FOREACH (it, songs) {
        if (CheckConditionsForSong(mgr, *it)) {
            num++;
        }
    }
    if (mSongCount == 0) {
        if (num >= songs.size()) {
            return true;
        }
    } else if (num >= mSongCount) {
        return true;
    }
    return false;
}

bool AccomplishmentDiscSongConditional::IsRelevantForSong(Symbol s) const {
    auto &songs = TheAccomplishmentMgr->GetDiscSongs();
    FOREACH (it, songs) {
        if (*it == s)
            return true;
    }
    return false;
}

bool AccomplishmentDiscSongConditional::InqIncrementalSymbols(
    HamProfile *, std::vector<Symbol> &discSongs
) const {
    discSongs = TheAccomplishmentMgr->GetDiscSongs();
    return true;
}

int AccomplishmentDiscSongConditional::GetNumCompletedSongs(HamProfile *profile) const {
    auto &songs = TheAccomplishmentMgr->GetDiscSongs();
    SongStatusMgr *mgr = profile->GetSongStatusMgr();
    int num = 0;
    FOREACH (it, songs) {
        Symbol cur = *it;
        if (TheHamSongMgr.HasSong(cur, false) && CheckConditionsForSong(mgr, cur)) {
            num++;
        }
    }
    return num;
}

void AccomplishmentDiscSongConditional::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x20);
    mSongCount = 0;
    static Symbol song_count("song_count");
    i_pConfig->FindData(song_count, mSongCount, false);
}
