#pragma once
#include "HamStoreFilterProvider.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
#include "meta/StoreOffer.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/Instarank.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/PracticeChoosePanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIListSlot.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

class AppLabel : public HamLabel {
public:
    // Hmx::Object
    virtual ~AppLabel();
    // no, this is not an oversight
    // the StaticClassName for AppLabel is in fact, HamLabel
    OBJ_CLASSNAME(HamLabel);
    OBJ_SET_TYPE(HamLabel);
    virtual DataNode Handle(DataArray *, bool);
    // HamLabel
    virtual void SetCreditsText(DataArray *, UIListSlot *);

    NEW_OBJ(AppLabel)

    void SetUserName(const User *);
    void SetUserName(int);
    void SetAlbumName(Symbol);
    void SetInstarank(const Instarank *);
    void SetFromSongSelectNode(const NavListNode *);
    void SetFromGeneralSelectNode(const NavListNode *);
    void SetFromPlaylistSelectNode(const NavListNode *);
    void SetChallengerName(const char *);
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
    void SetStepMoveName(const StepMoves &);
    void SetStoreOfferName(const StoreOffer *);
    void SetStoreOfferArtist(const StoreOffer *);
    void SetStoreOfferAlbum(const StoreOffer *);
    void SetStoreOfferCost(const StoreOffer *);
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
    void SetEnrolledPlayerName(int);
    void SetStoreFilterName(HamStoreFilter const *);

protected:
    DataNode OnSetUserName(const DataArray *);

private:
    void SetTimeElapsedSince(unsigned int);
};
