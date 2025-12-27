#include "meta_ham/MetaPerformer.h"
#include "SkillsAwardList.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "game/HamUserMgr.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamNavProvider.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/MoveDir.h"
#include "hamobj/ScoreUtl.h"
#include "math/Rand.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/Utl.h"
#include "net_ham/DataMinerJobs.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "utl/Locale.h"
#include "utl/MakeString.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

bool CharConflict(Symbol s1, Symbol s2) {
    bool symsEqual = s1 == s2;
    if (TheGameMode->InMode("dance_battle", true)) {
        Symbol crew = s1 == gNullStr ? gNullStr : GetCrewForCharacter(s1);
        Symbol crew2 = s2 == gNullStr ? gNullStr : GetCrewForCharacter(s2);
        symsEqual |= crew == crew2;
    }
    return symsEqual;
}

Symbol GetUnlockedOutfit(Symbol s1) {
    if (!TheProfileMgr.IsContentUnlocked(s1)) {
        Symbol outfit = GetOutfitCharacter(s1);
        return GetCharacterOutfit(outfit, 0);
    } else
        return s1;
}

#pragma region MetaPerformer

MetaPerformerHook *MetaPerformer::sScriptHook;

MetaPerformer::MetaPerformer(const HamSongMgr &mgr, const char *)
    : mNumRestarts(0), mSongMgr(mgr), mInstarank(0), mNoFail(0), mGotNewHighScore(0),
      unk30(0), mGotNewBestStars(0), unk32(0), mGotMovesPassedBest(0),
      mUnlockedNoFlashcards(0), mCompletedSongWithNoFlashcards(0),
      mUnlockedMediumDifficulty(0), mUnlockedExpertDifficulty(0), unk38(0),
      mPracticeOverallScore(0), mMoveScored(0), mCheckMoveScored(1), mPlaylist(0),
      mPlaylistIndex(0), mPlaylistElapsedTime(0), unke0(0), mSkipPracticeWelcome(0),
      unke3(0) {
    mNumCompleted.reserve(100);
    mSkippedSongs.clear();
    mEnrollmentIndex[0] = -1;
    mEnrollmentIndex[1] = -1;
    unk29 = DateTime(0);
    mSkillsAwards = new SkillsAwardList();
}

MetaPerformer::~MetaPerformer() { delete mSkillsAwards; }

void MetaPerformer::HandleSkippedSong() { mSkippedSongs.insert(mPlaylistIndex); }
void MetaPerformer::HandleSongRestart() { mNumRestarts++; }
bool MetaPerformer::IsGameplayTimerRunning() const { return unk29.ToCode(); }
bool MetaPerformer::GetPlayedLongIntro(Symbol intro) {
    return mLongIntrosPlayed.count(intro) > 0;
}
void MetaPerformer::SetPlayedLongIntro(Symbol intro) { mLongIntrosPlayed.insert(intro); }

BEGIN_HANDLERS(MetaPerformer)
    HANDLE_ACTION(set_venue_pref, TheProfileMgr.SetVenuePreference(_msg->ForceSym(2)))
    HANDLE_EXPR(get_venue_pref, TheProfileMgr.GetVenuePreference())
    HANDLE_EXPR(has_instarank_data, HasInstarankData())
    HANDLE_EXPR(get_instarank, GetInstarank())
    HANDLE_ACTION(reset_songs, ResetSongs())
    HANDLE_EXPR(num_completed, (int)mNumCompleted.size())
    HANDLE_EXPR(
        get_completed_song_id,
        TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong(), true)
    )
    HANDLE_EXPR(get_song, GetSong())
    HANDLE_EXPR(is_first_song, mNumCompleted.empty())
    HANDLE_EXPR(is_last_song, IsLastSong())
    HANDLE_EXPR(is_set_complete, IsSetComplete())
    HANDLE_EXPR(is_winning, IsWinning())
    HANDLE_ACTION(handle_skipped_song, HandleSkippedSong())
    HANDLE_ACTION(restart, Restart())
    HANDLE_ACTION(handle_song_restart, HandleSongRestart())
    HANDLE_EXPR(cheat_toggle_finale, sCheatFinale = !sCheatFinale)
    HANDLE_ACTION(
        trigger_song_completion, TriggerSongCompletion(_msg->Int(2), _msg->Float(3))
    )
    HANDLE_ACTION(advance_song, AdvanceSong(_msg->Int(2)))
    HANDLE_EXPR(is_no_fail_active, mNoFail)
    HANDLE_ACTION(use_no_fail, mNoFail = _msg->Int(2))
    HANDLE_EXPR(is_possible_to_fail, !mNoFail)
    HANDLE_EXPR(is_no_flashcards_active, TheProfileMgr.NoFlashcards())
    HANDLE_EXPR(completed_song_with_no_flashcards, mCompletedSongWithNoFlashcards)
    HANDLE_EXPR(get_enrollment_index, mEnrollmentIndex[_msg->Int(2)])
    HANDLE_EXPR(is_gameplay_timer_running, IsGameplayTimerRunning())
    HANDLE_ACTION(jump_gameplay_timer_forward, JumpGameplayTimerForward(_msg->Int(2)))
    HANDLE_EXPR(got_new_high_score, mGotNewHighScore)
    HANDLE_EXPR(got_new_best_stars, mGotNewBestStars)
    HANDLE_EXPR(got_moves_passed_best, mGotMovesPassedBest)
    HANDLE_ACTION(game_init, OnGameInit())
    HANDLE_ACTION(game_reset, OnGameInit())
    HANDLE_ACTION(freestyle_picture_taken, OnFreestylePictureTaken())
    HANDLE_ACTION(
        move_passed,
        OnMovePassed(_msg->Int(2), _msg->Obj<HamMove>(3), _msg->Int(4), _msg->Float(5))
    )
    HANDLE_ACTION(
        practice_move_passed,
        OnPracticeMovePassed(
            _msg->Int(2), _msg->Str(3), (SkillsAward)_msg->Int(4), _msg->Int(5)
        )
    )
    HANDLE_ACTION(
        review_move_passed,
        OnReviewMovePassed(
            _msg->Int(2), _msg->Obj<HamMove>(3), _msg->Int(4), _msg->Float(5)
        )
    )
    HANDLE_EXPR(get_moves_passed, GetMovesPassed(_msg->Int(2)))
    HANDLE_EXPR(get_moves_attempted, mMovesAttempted[_msg->Int(2)])
    HANDLE_ACTION(
        recall_move_passed, OnRecallMovePassed(_msg->Int(2), _msg->Obj<HamMove>(2)) // huh
    )
    HANDLE_EXPR(is_endgame_song, 0)
    HANDLE_EXPR(just_beat_game, mJustBeatGame)
    HANDLE_EXPR(last_played_mode, mLastPlayedMode)
    HANDLE_EXPR(unlocked_no_flashcards, mUnlockedNoFlashcards)
    HANDLE_EXPR(unlocked_medium_difficulty, mUnlockedMediumDifficulty)
    HANDLE_EXPR(unlocked_expert_difficulty, mUnlockedExpertDifficulty)
    HANDLE_EXPR(is_difficulty_unlocked, IsDifficultyUnlocked(_msg->Sym(2)))
    HANDLE_ACTION(calculate_practice_results, CalculatePracticeResults())
    HANDLE_EXPR(get_learn_moves_passed_count, mNumLearnMovesPassed)
    HANDLE_EXPR(get_learn_moves_fastlaned_count, mNumLearnMovesFastLaned)
    HANDLE_EXPR(get_learn_moves_total, mNumLearnMovesTotal)
    HANDLE_EXPR(get_practice_learn_score, mPracticeLearnScore)
    HANDLE_EXPR(get_review_moves_passed_count, mNumReviewMovesPassed)
    HANDLE_EXPR(get_review_moves_total, mNumReviewMovesTotal)
    HANDLE_EXPR(get_practice_review_score, mPracticeReviewScore)
    HANDLE_EXPR(get_practice_overall_score, mPracticeOverallScore)
    HANDLE_EXPR(
        get_practice_overall_moves_passed_count,
        mNumLearnMovesPassed + mNumReviewMovesPassed
    )
    HANDLE_EXPR(
        get_practice_overall_moves_total, mNumLearnMovesTotal + mNumReviewMovesTotal
    )
    HANDLE_EXPR(get_played_long_intro, GetPlayedLongIntro(_msg->Str(2)))
    HANDLE_ACTION(set_played_long_intro, SetPlayedLongIntro(_msg->Str(2)))
    HANDLE_EXPR(get_default_crew_character, GetCrewCharacter(_msg->Sym(2), 0))
    HANDLE_EXPR(get_crew_character, GetCrewCharacter(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(get_crew_venue, GetCrewVenue(_msg->Sym(2)))
    HANDLE_EXPR(get_crew_for_character, GetCrewForCharacter(_msg->Sym(2)))
    HANDLE_EXPR(is_crew_available, IsCrewAvailable(_msg->Sym(2)))
    HANDLE_EXPR(is_move_scored, mMoveScored)
    HANDLE_ACTION(set_move_scored, mMoveScored = _msg->Int(2))
    HANDLE_EXPR(get_check_move_scored, mCheckMoveScored)
    HANDLE_ACTION(set_check_move_scored, mCheckMoveScored = _msg->Int(2))
    HANDLE_EXPR(has_playlist, HasPlaylist())
    HANDLE_EXPR(is_playlist_empty, IsPlaylistEmpty())
    HANDLE_EXPR(is_playlist_playable, IsPlaylistPlayable())
    HANDLE_EXPR(is_playlist_custom, IsPlaylistCustom())
    HANDLE_ACTION(set_playlist, SetPlaylist(_msg->Sym(2)))
    HANDLE_ACTION(set_default_crews, SetDefaultCrews())
    HANDLE_ACTION(start_playlist, StartPlaylist())
    HANDLE_ACTION(continue_playlist, ContinuePlaylist())
    HANDLE_ACTION(shuffle_playlist, ShufflePlaylist())
    HANDLE_ACTION(repeat_current_playlist_song, RepeatCurrentPlaylistSong())
    HANDLE_EXPR(get_num_songs_in_playlist, GetNumSongsInPlaylist())
    HANDLE_EXPR(get_playlist_index, GetPlaylistIndex())
    HANDLE_ACTION(set_playlist_index, SetPlaylistIndex(_msg->Int(2)))
    HANDLE_EXPR(get_playlist_elapsed_time_string, GetPlaylistElapsedTimeString())
    HANDLE_ACTION(
        populate_playlist_song_provider,
        PopulatePlaylistSongProvider(_msg->Obj<HamNavProvider>(2))
    )
    HANDLE_EXPR(get_playlist_name_and_duration, GetPlaylistNameAndDuration())
    HANDLE_EXPR(get_playlist_elapsed_time, mPlaylistElapsedTime)
    HANDLE_ACTION(
        generate_recommended_practice_moves,
        GenerateRecommendedPracticeMoves(_msg->Int(2))
    )
    HANDLE_EXPR(has_recommended_practice_moves, HasRecommendedPracticeMoves()) // fix
    HANDLE_EXPR(failed_any_practice_moves, mSkillsAwards->HasFailure())
    HANDLE_EXPR(get_award, mSkillsAwards->GetAward(_msg->Array(2)))
    HANDLE_ACTION(
        set_award, mSkillsAwards->SetAward(_msg->Array(2), (SkillsAward)_msg->Int(3))
    )
    HANDLE_ACTION(clear_skills_awards, mSkillsAwards->Clear())
    HANDLE_EXPR(should_skip_practice_welcome, mSkipPracticeWelcome)
    HANDLE_ACTION(set_skip_practice_welcome, mSkipPracticeWelcome = _msg->Int(2))
    HANDLE_EXPR(get_num_restarts, mNumRestarts)
    HANDLE_ACTION(send_omg_score_datapoint, SendOmgDatapoint(_msg->Int(2), _msg->Int(3)))
    HANDLE_ACTION(send_drop_in_datapoint, SendDropInDatapoint(_msg->Int(2)))
    HANDLE_ACTION(send_drop_out_datapoint, SendDropOutDatapoint(_msg->Int(2)))
    HANDLE_ACTION(
        send_speech_datapoint,
        SendSpeechDatapoint(_msg->Array(2), _msg->Float(3), _msg->Sym(4))
    )
    HANDLE_ACTION(get_era, GetEraInvalid())
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool MetaPerformer::IsSetComplete() const { return mNumCompleted.size() == 1; }
void MetaPerformer::RepeatCurrentPlaylistSong() {
    UpdateSongFromPlaylist();
    UpdateIsLastSong();
}

void MetaPerformer::SetPlaylistIndex(int idx) {
    mPlaylistIndex = idx;
    RepeatCurrentPlaylistSong();
}

bool MetaPerformer::HasRecommendedPracticeMoves() const {
    return mRecommendedPracticeMoves.size() > 0;
}

DataNode MetaPerformer::OnMsg(const RCJobCompleteMsg &) { return 1; }

void MetaPerformer::ResetSongs() {
    mNumCompleted.clear();
    mNumRestarts = 0;
    if (mInstarank) {
        RELEASE(mInstarank);
    }
}

MetaPerformer *MetaPerformer::Current() { return sScriptHook->Current(); }

bool MetaPerformer::CanUpdateScoreLeaderboards(bool b1) {
    if (TheGameMode) {
        if (TheGameMode->Property("update_leaderboards")->Int() == 0) {
            return false;
        }
    }
    return true;
}

void MetaPerformer::SetVenuePref(Symbol venue) {
    TheProfileMgr.SetVenuePreference(venue);
}

void MetaPerformer::SetSong(Symbol song) {
    mPlaylist = 0;
    ResetSongs();
    SelectSong(song, 0);
}

void MetaPerformer::StartGameplayTimer() {
    if (unk29.ToCode() == 0) {
        GetDateAndTime(unk29);
    }
}

void MetaPerformer::JumpGameplayTimerForward(int x) {
    if (unk29.ToCode() == 0) {
        MILO_NOTIFY("This cheat only works while the gameplay timer is running!");
    } else {
        unk29 = DateTime(unk29.ToCode() - x);
    }
}

void MetaPerformer::GetEraInvalid() {
    MILO_LOG("Unhandled 'get_era' request made to MetaPerformer.\n");
}

void MetaPerformer::CalcPrimarySongCharacter(
    const HamSongMetadata *data, Symbol &s2, Symbol &s3, Symbol &s4
) {
    s3 = data->Character();
    s4 = data->Outfit();
    s2 = GetCrewForCharacter(s3);
}

int MetaPerformer::GetPlaylistIndex() const { return mPlaylistIndex; }
Symbol MetaPerformer::GetCompletedSong() const { return TheGameData->GetSong(); }
bool MetaPerformer::SongInSet(Symbol song) const {
    return song == TheGameData->GetSong();
}

Symbol MetaPerformer::GetSong() const {
    MILO_ASSERT(TheGameData, 0x118);
    return TheGameData->GetSong();
}

bool MetaPerformer::IsLastSong() const {
    if (mPlaylist) {
        return mPlaylist->GetLastValidSongIndex() <= mPlaylistIndex;
    } else {
        return TheGameMode->IsInfinite() == false;
    }
}

void MetaPerformer::StopGameplayTimer() {
    if (unk29.ToCode()) {
        unsigned int curCode = unk29.ToCode();
        DateTime now;
        GetDateAndTime(now);
        unsigned int nowCode = now.ToCode();
        for (int i = 0; i < 2; i++) {
            HamPlayerData *pPlayer = TheGameData->Player(i);
            MILO_ASSERT(pPlayer, 0x35E);
            HamProfile *pProfile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
            if (pProfile && pProfile->HasValidSaveData()) {
                // metagamestats
            }
        }
        unk29 = DateTime(0);
    }
}

void MetaPerformer::OnFreestylePictureTaken() {
    for (int i = 0; i < 6; i++) {
        TheGestureMgr->GetSkeleton(i);
    }
}

int MetaPerformer::GetMovesPassed(int i1) {
    int num = -1;
    int numMoves = unk40[i1].size();
    if (numMoves) {
        int loop_moves = 0;
        for (int i = 0; i < unk40[i1].size(); i++) {
            static Symbol move_perfect("move_perfect");
            static Symbol move_awesome("move_awesome");
            Symbol rating = RatingState(i);
            if (rating == move_perfect || rating == move_awesome) {
                loop_moves++;
            }
        }
        num = loop_moves / numMoves;
    }
    return num;
}

bool MetaPerformer::IsCheatWinning() const { return sCheatFinale && IsLastSong(); }

void MetaPerformer::ClearCharacters() {
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1Data, 0x5B9);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2Data, 0x5BB);
    pPlayer1Data->SetCrew(gNullStr);
    pPlayer2Data->SetCrew(gNullStr);
    pPlayer1Data->SetCharacter(gNullStr);
    pPlayer2Data->SetCharacter(gNullStr);
}

bool MetaPerformer::IsCrewAvailable(Symbol crew) const {
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1Data, 0x699);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2Data, 0x69C);
    if (pPlayer1Data->Crew() == crew) {
        return false;
    } else {
        return pPlayer2Data->Crew() != crew;
    }
}

Symbol MetaPerformer::GetCrewVenue(Symbol s) const {
    static Symbol CREWS("CREWS");
    DataArray *pCrewArray = DataGetMacro(CREWS);
    MILO_ASSERT(pCrewArray, 0x6B0);
    DataArray *pCrewData = pCrewArray->FindArray(s);
    MILO_ASSERT(pCrewData, 0x6B3);
    static Symbol venue("venue");
    return pCrewData->FindSym(venue);
}

bool MetaPerformer::IsPlaylistEmpty() const {
    MILO_ASSERT(mPlaylist, 0x6BC);
    return mPlaylist->GetNumSongs() == 0;
}

bool MetaPerformer::IsPlaylistPlayable() const {
    MILO_ASSERT(mPlaylist, 0x6C7);
    return mPlaylist->GetLastValidSongIndex() >= 0;
}

bool MetaPerformer::IsPlaylistCustom() const {
    MILO_ASSERT(mPlaylist, 0x6CE);
    return mPlaylist->IsCustom();
}

int MetaPerformer::GetNumSongsInPlaylist() const {
    MILO_ASSERT(mPlaylist, 0x758);
    return mPlaylist->GetNumSongs();
}

bool MetaPerformer::SongEndsWithEndgameSequence() const {
    return IsWinning() && IsLastSong();
}

bool MetaPerformer::IsDifficultyUnlocked(Symbol s) const {
    if (mPlaylist && IsPlaylistPlayable()) {
        int numSongs = GetNumSongsInPlaylist();
        for (int i = 0; i < numSongs; i++) {
            if (mPlaylist->IsValidSong(i)) {
                int song = mPlaylist->GetSong(i);
                Symbol shortname = TheHamSongMgr.GetShortNameFromSongID(song);
                if (!TheProfileMgr.IsDifficultyUnlocked(shortname, s)) {
                    return false;
                }
            }
        }
        return true;
    } else {
        Symbol song = GetSong();
        return TheProfileMgr.IsDifficultyUnlocked(song, s);
    }
}

int MetaPerformer::DetermineDanceBattleWinner() {
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1Data, 0x307);
    Hmx::Object *pPlayer1Provider = pPlayer1Data->Provider();
    MILO_ASSERT(pPlayer1Provider, 0x309);
    static Symbol score("score");
    const DataNode *pPlayer1Score = pPlayer1Provider->Property(score);
    MILO_ASSERT(pPlayer1Score, 0x30C);
    int p1ScoreValue = pPlayer1Score->Int();
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2Data, 0x311);
    Hmx::Object *pPlayer2Provider = pPlayer2Data->Provider();
    MILO_ASSERT(pPlayer2Provider, 0x313);
    const DataNode *pPlayer2Score = pPlayer2Provider->Property(score);
    MILO_ASSERT(pPlayer2Score, 0x315);
    int p2ScoreValue = pPlayer2Score->Int();
    if (p2ScoreValue < p1ScoreValue) {
        return 0;
    } else if (p1ScoreValue < p2ScoreValue) {
        return 1;
    } else
        return -1;
}

void MetaPerformer::PotentiallyUpdateLeaderboards(
    bool b1, Symbol s2, int i3, int i4, bool b5
) {
    if (TheHamUserMgr && !b1 && CanUpdateScoreLeaderboards(b5)) {
        SaveAndUploadScores(s2, i3, i4);
    }
}

bool MetaPerformer::IsRecommendedPracticeMove(String move) const {
    for (int i = 0; i < mRecommendedPracticeMoves.size(); i++) {
        if (mRecommendedPracticeMoves[i] == move) {
            return true;
        }
    }
    return false;
}

bool MetaPerformer::IsRecommendedPracticeMoveGroup(
    const std::vector<class HamMove *> &moves
) const {
    DataArrayPtr ptr(new DataArray(moves.size()));
    for (int i = 0; i < moves.size(); i++) {
        ptr->Node(i) = moves[i];
    }
    return mSkillsAwards->GetAward(ptr) == 1;
}

void MetaPerformer::SetDefaultCrews() {
    if (!TheGameMode->InMode("campaign", true)) {
        ClearCharacters();
        SetupCharacters();
    }
}

void MetaPerformer::UpdateIsLastSong() {
    bool last = IsLastSong();
    static Symbol is_last_song("is_last_song");
    TheHamProvider->SetProperty(is_last_song, last);
}

void MetaPerformer::SetPlaylist(Playlist *playlist) {
    ResetSongs();
    if (unke3) {
        RELEASE(mPlaylist);
        unke3 = false;
    }
    mPlaylist = playlist;
    mPlaylistIndex = 0;
    mSkippedSongs.clear();
    UpdateIsLastSong();
}

void MetaPerformer::StartPlaylist() {
    mPlaylistElapsedTime = 0;
    if (!TheGameMode->IsInfinite()) {
        MILO_ASSERT(mPlaylist, 0x713);
        MILO_ASSERT(!mPlaylist->IsEmpty(), 0x714);
        mPlaylistIndex = 0;
        mSkippedSongs.clear();
        while (!mPlaylist->IsValidSong(mPlaylistIndex)) {
            mPlaylistIndex++;
            MILO_ASSERT(mPlaylistIndex < mPlaylist->GetNumSongs(), 0x71D);
        }
    }
    if (mPlaylist->GetDuration() >= 900) {
        unke0 = true;
    }
    UpdateSongFromPlaylist();
    UpdateIsLastSong();
}

void MetaPerformer::ContinuePlaylist() {
    if (TheGameMode->IsInfinite()
        || TheHamProvider->Property("is_in_infinite_party_mode")->Int()) {
        if (!TheGameMode->InMode("campaign", true))
            goto end;
    }
    MILO_ASSERT(mPlaylist, 0x731);
    mPlaylistIndex++;
    while (!mPlaylist->IsValidSong(mPlaylistIndex)) {
        mSkippedSongs.insert(mPlaylistIndex);
        mPlaylistIndex++;
        MILO_ASSERT(mPlaylistIndex < mPlaylist->GetNumSongs(), 0x73B);
    }
end:
    UpdateSongFromPlaylist();
    UpdateIsLastSong();
}

void MetaPerformer::ShufflePlaylist() {
    if (!unke3) {
        mPlaylist = new Playlist(*mPlaylist);
        mPlaylist->ShuffleSongs();
        unke3 = true;
    }
}

void MetaPerformer::SetPlaylist(Symbol s) {
    Playlist *pPlaylist = TheHamSongMgr.GetPlaylist(s);
    MILO_ASSERT(pPlaylist, 0x6D5);
    SetPlaylist(pPlaylist);
}

void MetaPerformer::Restart() {
    mNumCompleted.clear();
    mNumRestarts++;
    if (mInstarank) {
        RELEASE(mInstarank);
    }
}

Symbol MetaPerformer::GetRandomVenue() {
    static Symbol review("review");
    std::vector<Symbol> validVenues;
    DataArray *venueArray = SystemConfig()->FindArray("venues", false);
    MILO_ASSERT(venueArray, 0x25E);
    for (int i = 1; i < venueArray->Size(); i++) {
        DataArray *pVenueEntryArray = venueArray->Array(i);
        MILO_ASSERT(pVenueEntryArray, 0x263);
        static Symbol never_show("never_show");
        bool neverShow = false;
        pVenueEntryArray->FindData(never_show, neverShow, false);
        if (!neverShow) {
            Symbol s = pVenueEntryArray->Sym(0);
            if (TheProfileMgr.IsContentUnlocked(s) && s != review) {
                validVenues.push_back(s);
            }
        }
    }
    MILO_ASSERT(validVenues.size() > 0, 0x272);
    return validVenues[RandomInt(0, validVenues.size())];
}

void MetaPerformer::OnPracticeMovePassed(
    int playerIndex, const char *cc, SkillsAward award, bool b4
) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x41A);
    HamPlayerData *pPlayerData = TheGameData->Player(playerIndex);
    MILO_ASSERT(pPlayerData, 0x41D);
    HamMove *theMove = nullptr;
    MoveDir *moveDir = TheHamDirector->GetWorld()->Find<MoveDir>("moves", true);
    std::vector<HamMoveKey> keys;
    TheHamDirector->MoveKeys(pPlayerData->GetDifficulty(), moveDir, keys);
    int numKeys = keys.size();
    for (int i = 0; i != numKeys; i++) {
        if (streq(keys[i].move->Name(), cc)) {
            theMove = keys[i].move;
        }
    }
    if (unk40[playerIndex].size() == 0) {
        unk40[playerIndex].reserve(numKeys);
    }
    HamMoveScore score;
    int i4 = -1;
    switch (award) {
    case 1:
        i4 = -2;
        break;
    case 2:
        i4 = -3;
        break;
    case 3:
        i4 = -4;
        break;
    default:
        break;
    }
    score.unk0 = theMove;
    score.unk4 = i4;
    score.unk8 = 0;
    score.unkc = b4;
    unk40[playerIndex].push_back(score);
    mMovesAttempted[playerIndex]++;
}

void MetaPerformer::Init() {
    MILO_ASSERT(!sScriptHook, 0xC1);
    sScriptHook = new MetaPerformerHook(TheHamSongMgr);
}

void MetaPerformer::GenerateRecommendedPracticeMoves(int player) {
    MILO_ASSERT(player>=0 && player < MULTIPLAYER_SLOTS, 0x4D4);
    ClearAndShrink(mRecommendedPracticeMoves);
    for (int i = 0; i < unk40[player].size(); i++) {
        String name = unk40[player][i].unk0->DisplayName();
        if (!IsRecommendedPracticeMove(name)) {
            if (CheckRecommendedPracticeMove(name, player)) {
                mRecommendedPracticeMoves.push_back(name);
            }
        }
    }
}

void MetaPerformer::SendSpeechDatapoint(DataArray *a, float confidence, Symbol rule) {
    static Symbol sr_confidence("sr_confidence");
    static Symbol sr_tag("sr_tag");
    static Symbol sr_rule("sr_rule");
    static Symbol sr_language("sr_language");
    static Symbol sr_locale("sr_locale");
    if (a->Size() > 0) {
        SendDataPoint(
            "speech",
            sr_rule,
            rule,
            sr_tag,
            a->Sym(0),
            sr_confidence,
            confidence,
            sr_language,
            SystemLanguage(),
            sr_locale,
            SystemLocale()
        );
    }
}

#pragma endregion
#pragma region QuickplayPerformer

QuickplayPerformer::QuickplayPerformer(const HamSongMgr &mgr)
    : MetaPerformer(mgr, "quickplay_performer") {}

BEGIN_HANDLERS(QuickplayPerformer)
    HANDLE(set_song, OnSetSong)
    HANDLE_ACTION(setup_venue, ChooseVenue())
    HANDLE_SUPERCLASS(MetaPerformer)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void QuickplayPerformer::SelectSong(Symbol song, int) {
    TheGameData->SetSong(song);
    if (mNumCompleted.empty()) {
        SetDefaultCrews();
        ChooseVenue();
    }
}

void QuickplayPerformer::ChooseVenue() {
    Symbol preferredVenue = TheProfileMgr.GetVenuePreference();
    static Symbol random_venue("random_venue");
    static Symbol defaultSym("default");
    if (preferredVenue == random_venue || preferredVenue == gNullStr) {
        TheGameData->SetVenue(GetRandomVenue());
    } else if (preferredVenue == defaultSym) {
        const HamSongMetadata *pData = TheHamSongMgr.Data(
            TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong(), false)
        );
        MILO_ASSERT(pData, 0x7A);
        TheGameData->SetVenue(pData->Venue());
    } else {
        TheGameData->SetVenue(preferredVenue);
    }
}

DataNode QuickplayPerformer::OnSetSong(DataArray *a) {
    SetSong(a->ForceSym(2));
    return 0;
}

void MetaPerformer::SendOmgDatapoint(int p1Score, int p2Score) {
    TheRockCentral.ManageJob(new OmgScoresJob(nullptr, p1Score, p2Score));
}

void MetaPerformer::SendDropInDatapoint(int playerIdx) {
    TheRockCentral.ManageJob(new PlayerDroppedInJob(nullptr, playerIdx));
}

void MetaPerformer::SendDropOutDatapoint(int playerIdx) {
    TheRockCentral.ManageJob(new PlayerDroppedOutJob(nullptr, playerIdx));
}

String MetaPerformer::GetPlaylistElapsedTimeString() const {
    if (!mPlaylist && !TheGameMode->IsInfinite())
        return "";
    else
        return FormatTimeMS(mPlaylistElapsedTime);
}

String MetaPerformer::GetPlaylistNameAndDuration() const {
    if (!mPlaylist)
        return "";
    else {
        mPlaylist->GetDuration();
        return MakeString(
            "%s (%s)",
            Localize(mPlaylist->GetName(), false, TheLocale),
            FormatTimeMS(mPlaylist->GetDuration())
        );
    }
}

#pragma endregion
#pragma region MetaPerformerHook

MetaPerformerHook::~MetaPerformerHook() { delete mQuickplayPerformer; }
