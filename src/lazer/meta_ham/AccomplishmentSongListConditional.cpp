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
    : AccomplishmentSongConditional(d, i), unk7c(0) {
    Configure(d);
}

AccomplishmentSongListConditional::~AccomplishmentSongListConditional() {}

int AccomplishmentSongListConditional::GetTotalNumSongs() const { return unk70.size(); }

bool AccomplishmentSongListConditional::InqIncrementalSymbols(
    HamProfile *hp, std::vector<Symbol> &vec
) const {
    vec = unk70;
    return true;
}

void AccomplishmentSongListConditional::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x1f);
    static Symbol songs("songs");
    DataArray *pSongArray = i_pConfig->FindArray(songs);
    MILO_ASSERT(pSongArray->Size() > 1, 0x26);
    for (int i = 1; i < pSongArray->Size(); i++) {
        Symbol s = pSongArray->Node(i).Sym(0);
        unk70.push_back(s);
    }
    unk7c = 0;
    static Symbol song_count("song_count");
    i_pConfig->FindData(song_count, unk7c, false);
    if (unk7c == 0) {
        unk7c = unk70.size();
    }
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

bool AccomplishmentSongListConditional::IsFulfilled(HamProfile *hp) const {
    SongStatusMgr *pSongStatusMgr = hp->GetSongStatusMgr();
    int i = 0;
    FOREACH (it, unk70) {
        if (CheckConditionsForSong(pSongStatusMgr, *it)) {
            i++;
        }
    }
    return i >= unk7c;
}

bool AccomplishmentSongListConditional::IsRelevantForSong(Symbol s) const {
    FOREACH (it, unk70) {
        if (*it == s)
            return true;
    }
    return false;
}
