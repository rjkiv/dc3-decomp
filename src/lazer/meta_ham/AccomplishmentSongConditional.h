#pragma once
#include "AccomplishmentConditional.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "utl/Symbol.h"

class AccomplishmentSongConditional : public AccomplishmentConditional {
public:
    AccomplishmentSongConditional(DataArray *, int);
    virtual ~AccomplishmentSongConditional();
    virtual bool ShowBestAfterEarn() const { return false; }
    virtual void UpdateIncrementalEntryName(UILabel *, Symbol);
    virtual bool InqProgressValues(HamProfile *, int &, int &);
    virtual bool IsSymbolEntryFulfilled(HamProfile *, Symbol) const;

protected:
    virtual int GetNumCompletedSongs(HamProfile *) const = 0;
    virtual int GetTotalNumSongs() const = 0;
    virtual bool CheckConditionsForSong(SongStatusMgr *, Symbol) const;

    bool
    CheckStarsCondition(SongStatusMgr *, Symbol, AccomplishmentCondition const &) const;
    bool
    CheckScoreCondition(SongStatusMgr *, Symbol, AccomplishmentCondition const &) const;
    bool CheckPracticePercentageCondition(
        SongStatusMgr *, Symbol, AccomplishmentCondition const &
    ) const;
    bool CheckNoFlashcardsCondition(SongStatusMgr *, Symbol) const;
};
