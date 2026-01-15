#pragma once
#include "gesture/FitnessFilter.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamNavProvider.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/PracticeSection.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/Instarank.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/SkillsAwardList.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"

#define MULTIPLAYER_SLOTS 2

enum PlayerFlag {
};

enum EndGameResult {
};

DECLARE_MESSAGE(EndGameMsg, "end_game")
EndGameMsg(EndGameResult r) : Message(Type(), r) {}
EndGameResult Result() const { return (EndGameResult)mData->Int(2); }
END_MESSAGE

class MetaPerformer : public virtual Hmx::Object {
public:
    MetaPerformer(const HamSongMgr &, const char *);
    virtual ~MetaPerformer();
    virtual DataNode Handle(DataArray *, bool);
    // MetaPerformer
    virtual bool IsWinning() const = 0;
    virtual void Clear() { ResetSongs(); }
    virtual void ResetSongs();
    virtual void SelectSong(Symbol, int) = 0;
    virtual void CompleteSong(int, int, int, float, bool);
    virtual void AdvanceSong(int i) { mNumCompleted.push_back(i); }
    virtual bool HasInstarankData() { return mInstarank; }
    virtual Instarank *GetInstarank() { return mInstarank; }
    virtual void OnLoadSong() {}

    Symbol GetSong() const;
    bool IsLastSong() const;
    bool IsSetComplete() const;
    void Restart();
    void TriggerSongCompletion(int, float);
    void JumpGameplayTimerForward(int);
    int GetMovesPassed(int);
    bool IsDifficultyUnlocked(Symbol) const;
    void CalculatePracticeResults();
    Symbol GetCrewVenue(Symbol) const;
    bool IsCrewAvailable(Symbol) const;
    bool IsPlaylistEmpty() const;
    bool IsPlaylistPlayable() const;
    bool IsPlaylistCustom() const;
    void SetPlaylist(Symbol);
    void SetDefaultCrews(Symbol);
    void StartPlaylist();
    void ContinuePlaylist();
    void ShufflePlaylist();
    void RepeatCurrentPlaylistSong();
    void UpdateSongFromPlaylist();
    void UpdateIsLastSong();
    int GetNumSongsInPlaylist() const;
    void SetPlaylistIndex(int);
    String GetPlaylistElapsedTimeString() const;
    void PopulatePlaylistSongProvider(HamNavProvider *) const;
    String GetPlaylistNameAndDuration() const;
    void GenerateRecommendedPracticeMoves(int);
    void SendOmgDatapoint(int, int);
    void SendDropInDatapoint(int);
    void SendDropOutDatapoint(int);
    void HandleSkippedSong();
    void HandleSongRestart();
    bool IsGameplayTimerRunning() const;
    void SetPlayedLongIntro(Symbol);
    bool GetPlayedLongIntro(Symbol);
    void SetDefaultCrews();
    bool HasRecommendedPracticeMoves() const;
    void SetSong(Symbol);
    bool CanUpdateScoreLeaderboards(bool);
    void SetVenuePref(Symbol);
    void StartGameplayTimer();
    void CalcPrimarySongCharacter(
        const HamSongMetadata *data, Symbol &crew, Symbol &charSym, Symbol &outfit
    );
    void CalcSecondarySongCharacter(
        const HamSongMetadata *, bool, Symbol, Symbol &, Symbol &, Symbol &
    );
    int GetPlaylistIndex() const;
    Symbol GetCompletedSong() const;
    bool SongInSet(Symbol) const;
    void StopGameplayTimer();
    void ClearCharacters();
    void CalcCharacters(
        const HamSongMetadata *,
        bool,
        PlayerFlag,
        HamPlayerData *&,
        Symbol &,
        Symbol &,
        Symbol &,
        HamPlayerData *&,
        Symbol &,
        Symbol &,
        Symbol &
    );
    bool SongEndsWithEndgameSequence() const;
    int DetermineDanceBattleWinner();
    bool IsRecommendedPracticeMove(String) const;
    bool IsRecommendedPracticeMoveGroup(const std::vector<class HamMove *> &) const;
    void SetDefaultSongCharacter(int);
    void SetupCharacters();
    void SetPlaylist(Playlist *);
    void HandleGameplayEnded(const EndGameResult &);
    void CheckForFitnessAccomplishments();
    int GetMovesPassedByType(int, Symbol);

    bool HasPlaylist() const { return mPlaylist; }
    Playlist *GetPlaylist() { return mPlaylist; }
    int GetPlaylistElapsedTime() const { return mPlaylistElapsedTime; }
    int GetUnk38() const { return unk38; }

    void SetSkipPracticeWelcome(bool b) { mSkipPracticeWelcome = b; }

    static void Init();
    static void SendSpeechDatapoint(DataArray *, float, Symbol);
    static MetaPerformer *Current();

private:
    void SaveAndUploadScores(Symbol, int, int);
    void SaveDanceBattleScores(Symbol);

    static bool sCheatFinale;
    static class MetaPerformerHook *sScriptHook;

protected:
    virtual void OnMovePassed(int, HamMove *, int, float);

    void OnGameInit();
    void OnFreestylePictureTaken();
    void OnPracticeMovePassed(int, const char *, SkillsAward, bool);
    void OnReviewMovePassed(int, HamMove *, int, float);
    void OnRecallMovePassed(int, HamMove *);
    void GetEraInvalid();
    bool IsCheatWinning() const;
    Symbol GetRandomVenue();
    void PotentiallyUpdateLeaderboards(bool, Symbol, int, int, bool);
    bool CheckRecommendedPracticeMove(String, int) const;
    void SetUpRecapResults();
    const std::vector<PracticeStep> &GetPracticeSteps() const;
    void GetCurrentRecapMove(int &, int &) const;

    DataNode OnMsg(const RCJobCompleteMsg &);

    int mEnrollmentIndex[2]; // 0x8
    std::vector<int> mNumCompleted; // 0x10
    int mNumRestarts; // 0x1c
    const HamSongMgr &mSongMgr; // 0x20
    Instarank *mInstarank; // 0x24
    bool mNoFail; // 0x28
    DateTime unk29; // 0x29
    bool mGotNewHighScore; // 0x2f
    bool unk30; // 0x30
    bool mGotNewBestStars; // 0x31
    bool unk32; // 0x32
    bool mGotMovesPassedBest; // 0x33
    bool mUnlockedNoFlashcards; // 0x34
    bool mCompletedSongWithNoFlashcards; // 0x35
    bool mUnlockedMediumDifficulty; // 0x36
    bool mUnlockedExpertDifficulty; // 0x37
    int unk38; // 0x38
    int unk3c; // 0x3c
    std::vector<HamMoveScore> mMoveScores[2]; // 0x40
    int mMovesAttempted[2]; // 0x58
    Symbol mLastPlayedMode; // 0x60
    std::vector<String> mRecommendedPracticeMoves; // 0x64
    SkillsAwardList *mSkillsAwards; // 0x70
    std::vector<std::vector<bool> > unk74;
    int mNumLearnMovesPassed; // 0x80
    int mNumLearnMovesFastLaned; // 0x84
    int mNumLearnMovesTotal; // 0x88
    int mPracticeLearnScore; // 0x8c
    int mNumReviewMovesPassed; // 0x90
    int mNumReviewMovesTotal; // 0x94
    int mPracticeReviewScore; // 0x98
    int mPracticeOverallScore; // 0x9c
    bool mMoveScored; // 0xa0
    bool mCheckMoveScored; // 0xa1
    std::set<Symbol> mLongIntrosPlayed; // 0xa4
    Playlist *mPlaylist; // 0xbc
    std::set<int> mSkippedSongs; // 0xc0
    int mPlaylistIndex; // 0xd8
    int mPlaylistElapsedTime; // 0xdc
    bool unke0; // 0xe0
    bool mJustBeatGame; // 0xe1
    bool mSkipPracticeWelcome; // 0xe2
    bool unke3; // 0xe3
    FitnessFilter mFitnessFilters[2]; // 0xe4
};

class QuickplayPerformer : public MetaPerformer {
public:
    QuickplayPerformer(const HamSongMgr &);
    virtual ~QuickplayPerformer() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual bool IsWinning() const { return IsCheatWinning() != false; }
    virtual void SelectSong(Symbol, int);
    virtual void ChooseVenue();

    DataNode OnSetSong(DataArray *);
};

class CampaignPerformer;

class MetaPerformerHook : public Hmx::Object {
public:
    MetaPerformerHook(const HamSongMgr &);
    virtual ~MetaPerformerHook();
    virtual DataNode Handle(DataArray *, bool);

    MetaPerformer *Current();

protected:
    QuickplayPerformer *mQuickplayPerformer; // 0x2c
    CampaignPerformer *mCampaignPerformer; // 0x30
};
