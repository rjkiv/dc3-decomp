#pragma once
#include "AccomplishmentConditional.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "utl/Symbol.h"

class AccomplishmentSongConditional : public AccomplishmentConditional {
public:
    virtual ~AccomplishmentSongConditional();
    virtual void UpdateIncrementalEntryName(UILabel *, Symbol);
    virtual bool InqProgressValues(HamProfile *, int &, int &);
    virtual bool IsSymbolEntryFulfilled(HamProfile *, Symbol) const;

    AccomplishmentSongConditional(DataArray *, int);

protected:
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
