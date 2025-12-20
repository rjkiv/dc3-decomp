#include "meta_ham/AccomplishmentSongListConditional.h"
#include "AccomplishmentSongConditional.h"
#include "AccomplishmentSongListConditional.h"
#include "HamProfile.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "stl/_vector.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

AccomplishmentSongListConditional::AccomplishmentSongListConditional(DataArray *d, int i)
    : AccomplishmentSongConditional(d, i), mSongCount(0) {
    Configure(d);
}

AccomplishmentSongListConditional::~AccomplishmentSongListConditional() {}

bool AccomplishmentSongListConditional::IsFulfilled(HamProfile *hp) const {
    SongStatusMgr *pSongStatusMgr = hp->GetSongStatusMgr();
    int num = 0;
    FOREACH (it, unk70) {
        if (CheckConditionsForSong(pSongStatusMgr, *it)) {
            num++;
        }
    }
    if (mSongCount <= num) {
        return true;
    } else {
        return false;
    }
}

bool AccomplishmentSongListConditional::IsRelevantForSong(Symbol s) const {
    FOREACH (it, unk70) {
        if (*it == s)
            return true;
    }
    return false;
}

bool AccomplishmentSongListConditional::InqIncrementalSymbols(
    HamProfile *hp, std::vector<Symbol> &vec
) const {
    vec = unk70;
    return true;
}

int AccomplishmentSongListConditional::GetNumCompletedSongs(HamProfile *hp) const {
    SongStatusMgr *pSongStatusMgr = hp->GetSongStatusMgr();
    int songs = 0;
    FOREACH (it, unk70) {
        if (CheckConditionsForSong(pSongStatusMgr, *it))
            songs++;
    }
    return songs;
}

int AccomplishmentSongListConditional::GetTotalNumSongs() const { return unk70.size(); }

void AccomplishmentSongListConditional::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x1f);
    static Symbol songs("songs");
    DataArray *pSongArray = i_pConfig->FindArray(songs);
    MILO_ASSERT(pSongArray->Size() > 1, 0x26);
    for (int i = 1; i < pSongArray->Size(); i++) {
        Symbol s = pSongArray->Node(i).Sym(0);
        unk70.push_back(s);
    }
    mSongCount = 0;
    static Symbol song_count("song_count");
    i_pConfig->FindData(song_count, mSongCount, false);
    if (mSongCount == 0) {
        mSongCount = unk70.size();
    }
}
