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
    // no, this is not an oversight
    // the StaticClassName for AppLabel is in fact, HamLabel
    OBJ_CLASSNAME(HamLabel);
    OBJ_SET_TYPE(HamLabel);
    virtual DataNode Handle(DataArray *, bool);
    // HamLabel
    virtual void SetCreditsText(DataArray *, UIListSlot *);

    NEW_OBJ(AppLabel)

    void SetUserName(const User *user);
    void SetUserName(int padnum);
    void SetAlbumName(Symbol shortname);
    void SetInstarank(const Instarank *);
    void SetFromSongSelectNode(const NavListNode *);
    void SetFromGeneralSelectNode(const NavListNode *);
    void SetFromPlaylistSelectNode(const NavListNode *);
    void SetChallengerName(const char *);
    void SetLastPlayedScore(int songID);
    void SetBestPerformPerfect(int songID, Difficulty diff);
    void SetBestPerformNice(int songID, Difficulty diff);
    void SetTotalBattleWins(HamProfile *profile, int songID);
    void SetTotalBattleLosses(HamProfile *profile, int songID);
    void SetLocked(bool locked);
    void SetChecked(bool checked);
    void SetNew(bool isNew);
    void SetBuy(bool buy);
    void SetDownload(bool download);
    void SetRandomTip();
    void SetStepMoveName(const StepMoves &);
    void SetStoreOfferName(const StoreOffer *);
    void SetStoreOfferArtist(const StoreOffer *);
    void SetStoreOfferAlbum(const StoreOffer *);
    void SetStoreOfferCost(const StoreOffer *);
    void SetPlayerHighScore(int songID);
    void SetPlayerChallengeScore(int songID);
    void SetChallengeExp(int);
    void SetPotentialChallengeExp(int);
    void SetChallengerGamertag(int);
    void SetChallengeScore(int);
    void SetMedalCount(int);
    void SetSongDuration(Symbol shortname);
    void SetArtistName(Symbol shortname, bool);
    void SetChallengeScoreLabel(int);
    void SetBestPracticeDifficulty(int songID);
    void SetFitnessTimeNum(HamProfile *);
    void SetFitnessTotalCaloriesNum(HamProfile *);
    void SetPlaylistName(Playlist const *, bool, bool);
    void SetPackSongName(DataArray const *);
    void SetSongName(Symbol shortname, int, bool);
    void SetBlacklightSongName(Symbol shortname, int, bool);
    void SetPlaylistSongName(Symbol shortname, int, int);
    void SetDancer(Symbol shortname);
    void SetLastPracticeTime(int songID);
    void SetBestScore(int songID);
    void SetBestCoopScore(int songID);
    void SetBestPerformPercent(int songID, Difficulty diff);
    void SetBestBattleScore(HamProfile *profile, int songID);
    bool SetPracticeScore(int songID, Difficulty diff);
    void SetDiffScore(int songID, Difficulty diff);
    void SetFitnessTime(HamProfile *);
    void SetFitnessCalories(HamProfile *);
    void SetFitnessTotalCalories(HamProfile *);
    void SetExpireTime();
    void SetLastPlayedTime(int songID);
    void SetEnrolledPlayerName(int);
    void SetStoreFilterName(HamStoreFilter const *);

protected:
    DataNode OnSetUserName(const DataArray *);

private:
    void SetTimeElapsedSince(unsigned int);
};
