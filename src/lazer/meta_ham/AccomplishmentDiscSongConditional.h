#pragma once
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/AccomplishmentSongConditional.h"
#include "obj/Data.h"

class AccomplishmentDiscSongConditional : public AccomplishmentSongConditional {
public:
    AccomplishmentDiscSongConditional(DataArray *, int);
    virtual ~AccomplishmentDiscSongConditional();
    virtual AccomplishmentType GetType() const { return (AccomplishmentType)5; }
    virtual bool IsFulfilled(HamProfile *) const;
    virtual bool IsRelevantForSong(Symbol) const;
    virtual bool InqIncrementalSymbols(HamProfile *, std::vector<Symbol> &) const;
    virtual bool HasSpecificSongsToLaunch() const { return true; }

protected:
    virtual int GetNumCompletedSongs(HamProfile *) const;
    virtual int GetTotalNumSongs() const {
        return TheAccomplishmentMgr->GetDiscSongs().size();
    }

private:
    void Configure(DataArray *);

    std::vector<Symbol> unk70;
    int mSongCount; // 0x7c
};
