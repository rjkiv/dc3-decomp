#pragma once
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
#include "meta/StoreOffer.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/Instarank.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/Playlist.h"
#include "obj/Data.h"
#include "ui/UIListSlot.h"
#include "utl/Symbol.h"

class AppLabel : public HamLabel {
public:
    virtual DataNode Handle(DataArray *, bool);
    virtual void SetCreditsText(DataArray *, UIListSlot *);

    void SetUserName(User const *);
    void SetUserName(int);
    void SetAlbumName(Symbol);
    void SetInstarank(Instarank const *);
    void SetFromSongSelectNode(NavListNode const *);
    void SetFromGeneralSelectNode(NavListNode const *);
    void SetFromPlaylistSelectNode(NavListNode const *);
    void SetChallengerName(char const *);
    void SetLastPlayedScore(int);
    void SetBestPerformPerfect(int, Difficulty);
    void SetBestPerformNice(int, Difficulty);
    void SetTotalBattleWins(HamProfile *, int);
    void SetTotalBattleLosses(HamProfile *, int);
    void SetLocked(bool);
    void SetChecked(bool);
    void SetNew(bool);
    void SetBuy(bool);
    void SetDownload(bool);
    void SetRandomTip();
    // void SetStepMoveName(StepMoves const &);
    void SetStoreOfferName(StoreOffer const *);
    void SetStoreOfferArtist(StoreOffer const *);
    void SetStoreOfferAlbum(StoreOffer const *);
    void SetStoreOfferCost(StoreOffer const *);
    void SetPlayerHighScore(int);
    void SetPlayerChallengeScore(int);
    void SetChallengeExp(int);
    void SetPotentialChallengeExp(int);
    void SetChallengerGamertag(int);
    void SetChallengeScore(int);
    void SetMedalCount(int);
    void SetSongDuration(Symbol);
    void SetArtistName(Symbol, bool);
    void SetChallengeScoreLabel(int);
    void SetBestPracticeDifficulty(int);
    void SetFitnessTimeNum(HamProfile *);
    void SetFitnessTotalCaloriesNum(HamProfile *);
    void SetPlaylistName(Playlist const *, bool, bool);
    void SetPackSongName(DataArray const *);
    void SetSongName(Symbol, int, bool);
    void SetBlacklightSongName(Symbol, int, bool);
    void SetPlaylistSongName(Symbol, int, int);
    void SetDancer(Symbol);
    void SetLastPracticeTime(int);
    void SetBestScore(int);
    void SetBestCoopScore(int);
    void SetBestPerformPercent(int, Difficulty);
    void SetBestBattleScore(HamProfile *, int);
    bool SetPracticeScore(int, Difficulty);
    void SetDiffScore(int, Difficulty);
    void SetFitnessTime(HamProfile *);
    void SetFitnessCalories(HamProfile *);
    void SetFitnessTotalCalories(HamProfile *);
    void SetExpireTime();
    void SetLastPlayedTime(int);

protected:
    DataNode OnSetUserName(DataArray const *);

private:
    void SetTimeElapsedSince(unsigned int);
};
