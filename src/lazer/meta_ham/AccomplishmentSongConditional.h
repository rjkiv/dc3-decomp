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
    virtual void UpdateIncrementalEntryName(UILabel *label, Symbol shortname);
    virtual bool InqProgressValues(HamProfile *, int &, int &);
    virtual bool IsSymbolEntryFulfilled(HamProfile *profile, Symbol shortname) const;

protected:
    virtual int GetNumCompletedSongs(HamProfile *) const = 0;
    virtual int GetTotalNumSongs() const = 0;
    virtual bool CheckConditionsForSong(SongStatusMgr *, Symbol) const;

    bool CheckStarsCondition(
        SongStatusMgr *statusMgr,
        Symbol shortname,
        AccomplishmentCondition const &condition
    ) const;
    bool CheckScoreCondition(
        SongStatusMgr *statusMgr,
        Symbol shortname,
        AccomplishmentCondition const &condition
    ) const;
    bool CheckPracticePercentageCondition(
        SongStatusMgr *statusMgr,
        Symbol shortname,
        AccomplishmentCondition const &condition
    ) const;
    bool CheckNoFlashcardsCondition(SongStatusMgr *statusMgr, Symbol shortname) const;
};
