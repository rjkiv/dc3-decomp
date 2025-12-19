#pragma once
#include "AccomplishmentSongConditional.h"
#include "HamProfile.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

class AccomplishmentSongListConditional : public AccomplishmentSongConditional {
public:
    virtual ~AccomplishmentSongListConditional();
    virtual bool IsFulfilled(HamProfile *) const;
    virtual bool IsRelevantForSong(Symbol) const;
    virtual bool InqIncrementalSymbols(HamProfile *, std::vector<Symbol> &) const;

    AccomplishmentSongListConditional(DataArray *, int);

    std::vector<Symbol> unk70;
    int unk7c;

protected:
    virtual int GetNumCompletedSongs(HamProfile *) const;
    virtual int GetTotalNumSongs() const;

private:
    void Configure(DataArray *);
};
