#pragma once
#include "AccomplishmentSongConditional.h"
#include "HamProfile.h"
#include "meta_ham/Accomplishment.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

class AccomplishmentSongListConditional : public AccomplishmentSongConditional {
public:
    AccomplishmentSongListConditional(DataArray *, int);
    virtual ~AccomplishmentSongListConditional();
    virtual AccomplishmentType GetType() const { return (AccomplishmentType)1; }
    virtual bool IsFulfilled(HamProfile *) const;
    virtual bool IsRelevantForSong(Symbol) const;
    virtual bool InqIncrementalSymbols(HamProfile *, std::vector<Symbol> &) const;
    virtual bool HasSpecificSongsToLaunch() const { return true; }

protected:
    virtual int GetNumCompletedSongs(HamProfile *) const;
    virtual int GetTotalNumSongs() const;

private:
    void Configure(DataArray *);

    std::vector<Symbol> unk70;
    int mSongCount; // 0x7c
};
