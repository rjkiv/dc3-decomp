#pragma once
#include "hamobj/Difficulty.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/Playlist.h"
#include "obj/Data.h"
#include "utl/Symbol.h"

class CampaignPerformer : public MetaPerformer {
public:
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // MetaPerformer
    virtual void SelectSong(Symbol, int);
    virtual void OnLoadSong();
    virtual void OnMovePassed(int, HamMove *, int, float);

    CampaignPerformer(HamSongMgr const &);
    bool InOutroPerform() const;
    bool WonCurrentOutroSong() const;
    void SetDifficulty(Difficulty);
    void CheckForOutfitAwards(Difficulty, Symbol);
    void CheckForMasteryGoal(Difficulty, Symbol);
    int GetStarsRequiredForOutfits(Symbol) const;
    int GetSongStarsEarned(Symbol, Symbol) const;
    int GetStarsRequiredForMastery(Symbol) const;
    int GetMovesRequiredForMastery(Symbol) const;
    Symbol GetCompletionAccomplishment(Symbol) const;
    bool IsEraNew() const;
    bool IsCampaignNew() const;
    bool IsCampaignIntroComplete() const;
    void SetCampaignIntroComplete(bool);
    bool IsCampaignMindControlComplete() const;
    void SetCampaignMindControlComplete(bool);
    bool IsCampaignComplete() const;
    void SetCampaignComplete();
    bool IsEraMastered(Symbol) const;
    bool IsDanceCrazeSongAvailable(Symbol) const;
    bool IsEraComplete(Symbol) const;
    bool HasEraOutfits(Symbol) const;
    Symbol GetDanceCrazeSong() const;
    bool IsAttemptingDanceCrazeSong() const;
    Symbol GetEraSongUnlockedToken() const;
    Symbol GetEraCompleteToken() const;
    Symbol GetWinInstructionsToken() const;
    Symbol GetCharacterForSong() const;
    Symbol GetChallengeCharacter() const;
    Symbol GetEraIntroMovieToken() const;
    bool GetEraIntroMoviePlayed() const;
    void SetEraIntroMoviePlayed(bool);
    bool IsDanceCrazeMove(Symbol, Symbol, HamMove *);
    bool IsDanceCrazeMoveMastered(Symbol, Symbol, HamMove *);
    int GetNumEraSongs();
    Symbol GetEraSong(int);
    Symbol GetEraIntroSong();
    int GetSongIndex(Symbol);
    int GetNumSongCrazeMoves(Symbol);
    bool HasSongBeenAttempted(Symbol);
    bool CanSelectEraSong(Symbol);
    bool IsEraMoveMastered(Symbol, int);
    void BookmarkCurrentProgress();
    void ClearAllCampaignProgress();
    void UnlockAllMoves(Symbol, Symbol, int);
    void ResetAllCampaignProgress();
    void ClearSongProgress(Symbol, Symbol);
    Symbol GetFirstEra() const;
    Symbol GetLastEra() const;
    void SetOutroPlaylist();
    void UpdateEraSong(Difficulty, Symbol, Symbol, int);
    int GetEraStarsEarned(Symbol) const;
    int GetMasteryMoves(Symbol) const;
    void AwardCrazeAccomplishments();
    void AwardBossAccomplishment();
    void AwardMasterQuestAccomplishments();
    void UpdateStarsEarnedSoFar(int);
    int GetSongAttemptedCount();
    void SetupCampaignCharacters(Symbol, Symbol);
    void SetEra(Symbol);
    void SetIntroPlaylist();
    bool SetEraToFirstIncomplete();

    Symbol GetTanBattleEra() { return "era_tan_battle"; }

    Playlist mIntroPlaylist; // 0x114
    Playlist mOutroPlaylist; // 0x12c
    Symbol mEra; // 0x144
    Difficulty mDifficulty; // 0x148
    bool mJustFinishedEra; // 0x14c
    bool mJustUnlockedEraSong; // 0x14d
    bool mWasLastMoveMastered; // 0x14e
    Symbol mLastMoveMasteredName; // 0x150
    int mLastEraStars; // 0x154
    int mLastEraMoves; // 0x158
    int mStarsEarnedSoFar; // 0x15c

private:
    void CheckForEraSongUnlock();
};
