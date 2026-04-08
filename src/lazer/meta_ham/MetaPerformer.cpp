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
#include "hamobj/PracticeSection.h"
#include "hamobj/ScoreUtl.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SongStatusMgr.h"
#include "meta_ham/Utl.h"
#include "net_ham/DataMinerJobs.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "stl/_vector.h"
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
        s1 = GetCharacterOutfit(outfit, 0);
    }
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

void MetaPerformer::ResetSongs() {
    mNumCompleted.clear();
    mNumRestarts = 0;
    if (mInstarank) {
        RELEASE(mInstarank);
    }
}

void MetaPerformer::CompleteSong(int i1, int i2, int i3, float f4, bool b5) {
    Symbol song = TheGameData->GetSong();
    static Symbol campaign_outro("campaign_outro");
    if (TheGameMode->InMode(campaign_outro) || !TheGameMode->IsGameplayModePerform()) {
        if (TheGameMode->IsGameplayModeDanceBattle()) {
            TheHamSongMgr.GetSongIDFromShortName(song);
            if (TheHamUserMgr && !b5) {
                SaveDanceBattleScores(song);
            }
        }
    } else {
        TheHamSongMgr.GetSongIDFromShortName(song);
        PotentiallyUpdateLeaderboards(b5, song, i2, i1, false);
    }
    if (song != gNullStr) {
        mPlaylistElapsedTime += TheHamSongMgr.GetDuration(song);
    }
    CheckForFitnessAccomplishments();
    HandleGameplayEnded((EndGameResult)1);
    if (unke0 && mPlaylist) {
        if (IsLastSong() && IsPlaylistCustom()) {
            static Symbol acc_play_custom_play_list("acc_play_custom_play_list");
            TheAccomplishmentMgr->EarnAccomplishmentForAll(
                acc_play_custom_play_list, false
            );
        }
    }
    if (TheHamUserMgr) {
        TheAccomplishmentMgr->HandleSongCompleted(song);
    }
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x2DC);
        int padnum = pPlayer->PadNum();
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(padnum);
        if (profile) {
            TheAccomplishmentMgr->SetUnk30(padnum, false);
        }
    }
}

void MetaPerformer::OnMovePassed(
    int playerIndex, HamMove *move, int ratingIndex, float f4
) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x3EB);
    HamPlayerData *pPlayerData = TheGameData->Player(playerIndex);
    MILO_ASSERT(pPlayerData, 0x3EE);
    HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayerData->PadNum());
    static Symbol gameplay_mode("gameplay_mode");
    Symbol mode = TheGameMode->Property(gameplay_mode)->Sym();
    if (profile) {
        AccomplishmentProgress &progress = profile->AccessAccomplishmentProgress();
        progress.MovePassed(mode, ratingIndex);
    }
    mMovesAttempted[playerIndex]++;
    static Symbol perform("perform");
    static Symbol dance_battle("dance_battle");
    static Symbol bustamove("bustamove");
    static Symbol perform_legacy("perform_legacy");
    if (mode == perform || mode == dance_battle || mode == perform_legacy) {
        if (mMoveScores[playerIndex].size() == 0) {
            MoveDir *moves = TheHamDirector->GetWorld()->Find<MoveDir>("moves");
            std::vector<HamMoveKey> keys;
            TheHamDirector->MoveKeys(
                TheGameData->Player(playerIndex)->GetDifficulty(), moves, keys
            );
            mMoveScores[playerIndex].reserve(keys.size());
        }
        HamMoveScore score;
        score.unk8 = f4;
        score.unk0 = move;
        score.unk4 = ratingIndex;
        score.unkc = false;
        mMoveScores[playerIndex].push_back(score);
        if (profile) {
            profile->GetMoveRatingHistory()->AddHistory(move->DisplayName(), ratingIndex);
        }
        TheHamDirector->CheckBeginFatal(playerIndex, move, ratingIndex);
    }
}

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
    const HamSongMetadata *data, Symbol &crew, Symbol &charSym, Symbol &outfit
) {
    charSym = data->Character();
    outfit = data->Outfit();
    crew = GetCrewForCharacter(charSym);
}

void MetaPerformer::CalcSecondarySongCharacter(
    const HamSongMetadata *data, bool b2, Symbol s3, Symbol &s4, Symbol &s5, Symbol &s6
) {
    s6 = s3;
    s5 = GetOutfitCharacter(s6);
    s4 = GetCrewForCharacter(s5);
    if (b2) {
        Symbol rival = GetRivalOutfit(s6);
        Symbol outfitChar = GetOutfitCharacter(rival);
        if (!TheProfileMgr.IsContentUnlocked(outfitChar)
            || !TheProfileMgr.IsContentUnlocked(rival)) {
            rival = GetBackupRivalOutfit(s6);
        }
        s6 = rival;
        s5 = GetOutfitCharacter(s6);
    } else {
        s5 = GetAlternateCharacter(s5);
        s6 = TheProfileMgr.GetAlternateOutfit(s6);
    }
    s4 = GetCrewForCharacter(s5);
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
        return TheGameMode->Infinite() == false;
    }
}

void MetaPerformer::StopGameplayTimer() {
    if (unk29.ToCode()) {
        unsigned int curCode = unk29.ToCode();
        DateTime now;
        GetDateAndTime(now);
        unsigned int nowCode = now.ToCode();
        unsigned int timeDiff = nowCode - curCode;
        for (int i = 0; i < 2; i++) {
            HamPlayerData *pPlayer = TheGameData->Player(i);
            MILO_ASSERT(pPlayer, 0x35E);
            HamProfile *pProfile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
            if (pProfile && pProfile->HasValidSaveData()) {
                pProfile->GetMetagameStats()->WriteTimePlayed(pProfile, timeDiff);
            }
        }
        unk29 = DateTime(0);
    }
}

void MetaPerformer::OnFreestylePictureTaken() {
    for (int i = 0; i < NUM_SKELETONS; i++) {
        TheGestureMgr->GetSkeleton(i);
    }
}

int MetaPerformer::GetMovesPassed(int i1) {
    int num = -1;
    if (mMoveScores[i1].size()) {
        int loop_moves = 0;
        for (int i = 0; i < mMoveScores[i1].size(); i++) {
            static Symbol move_perfect("move_perfect");
            static Symbol move_awesome("move_awesome");
            Symbol rating = RatingState(mMoveScores[i1][i].unk4);
            if (rating == move_perfect || rating == move_awesome) {
                loop_moves++;
            }
        }
        num = (loop_moves * 100) / mMoveScores[i1].size();
    }
    return num;
}

int MetaPerformer::GetMovesPassedByType(int i1, Symbol s2) {
    int numPassed = 0;
    if (mMoveScores[i1].size()) {
        for (int i = 0; i < mMoveScores[i1].size(); i++) {
            Symbol rating = RatingState(mMoveScores[i1][i].unk4);
            if (rating == s2) {
                numPassed++;
            }
        }
    }
    return numPassed;
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
    if (!TheGameMode->Infinite()) {
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
    int infinite = TheGameMode->Infinite();
    bool infiniteParty = TheHamProvider->Property("is_in_infinite_party_mode")->Int();
    if ((!infinite && !infiniteParty) || TheGameMode->InMode("campaign")) {
        MILO_ASSERT(mPlaylist, 0x731);
        mPlaylistIndex++;
        while (!mPlaylist->IsValidSong(mPlaylistIndex)) {
            mSkippedSongs.insert(mPlaylistIndex);
            mPlaylistIndex++;
            MILO_ASSERT(mPlaylistIndex < mPlaylist->GetNumSongs(), 0x73B);
        }
    }
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
    for (int i = 0; i < keys.size(); i++) {
        if (streq(keys[i].move->Name(), cc)) {
            theMove = keys[i].move;
        }
    }
    if (mMoveScores[playerIndex].size() == 0) {
        mMoveScores[playerIndex].reserve(keys.size());
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
    mMoveScores[playerIndex].push_back(score);
    mMovesAttempted[playerIndex]++;
}

void MetaPerformer::Init() {
    MILO_ASSERT(!sScriptHook, 0xC1);
    sScriptHook = new MetaPerformerHook(TheHamSongMgr);
}

void MetaPerformer::GenerateRecommendedPracticeMoves(int player) {
    MILO_ASSERT(player>=0 && player < MULTIPLAYER_SLOTS, 0x4D4);
    ClearAndShrink(mRecommendedPracticeMoves);
    for (int i = 0; i < mMoveScores[player].size(); i++) {
        String name = mMoveScores[player][i].unk0->DisplayName();
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
    if (!mPlaylist && !TheGameMode->Infinite())
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

void MetaPerformer::TriggerSongCompletion(int i1, float f2) {
    if (!mPlaylist) {
        unke0 = false;
    }
    ThePlatformMgr.RemoveSink(TheAccomplishmentMgr);
    mJustBeatGame = false;
    static Symbol skipped_song("skipped_song");
    const DataNode *pSkippedSongNode = TheHamProvider->Property(skipped_song, false);
    MILO_ASSERT(pSkippedSongNode, 0x286);
    bool skipped = pSkippedSongNode->Int();
    if (i1 >= 0 && !skipped) {
        CompleteSong((int)f2, i1, i1, f2, false);
    }
}

void MetaPerformer::CheckForFitnessAccomplishments() {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayerData = TheGameData->Player(i);
        MILO_ASSERT(pPlayerData, 0x2EC);
        int pad = pPlayerData->PadNum();
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(pad);
        if (profile && profile->InFitnessMode() && !TheAccomplishmentMgr->Unk30(pad)) {
            float f88, f8c;
            if (mFitnessFilters[i].GetFitnessDataAndReset(f88, f8c)) {
                profile->SetFitnessStats(i, f88, f8c);
            }
            if (profile->IsFitnessDaysGoalMet() && profile->IsFitnessCaloriesGoalMet()) {
                static Symbol acc_weekly_goal("acc_weekly_goal");
                TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                    profile, acc_weekly_goal, false
                );
            }
        }
    }
}

void MetaPerformer::SetupCharacters() {
    Symbol song = TheGameData->GetSong();
    Symbol s38;
    Symbol s40;
    Symbol s3c;
    Symbol s2c;
    Symbol s34;
    Symbol s30;
    int songID = TheHamSongMgr.GetSongIDFromShortName(song);
    const HamSongMetadata *pSongData = TheHamSongMgr.Data(songID);
    MILO_ASSERT(pSongData, 0x68B);
    bool b5 = TheGameMode->InMode("dance_battle") || TheGameMode->InMode("strike_a_pose");
    HamPlayerData *p1;
    HamPlayerData *p2;
    CalcCharacters(pSongData, b5, (PlayerFlag)3, p1, s38, s40, s3c, p2, s2c, s34, s30);
    p1->SetCharacter(s40);
    p1->SetOutfit(s3c);
    p1->SetCrew(s38);
    p2->SetCharacter(s2c);
    p2->SetOutfit(s34);
    p2->SetCrew(s30);
}

void MetaPerformer::OnGameInit() {
    std::vector<HamProfile *> profiles = TheProfileMgr.GetSignedInProfiles();
    FOREACH (it, profiles) {
        HamProfile *profile = *it;
        MILO_ASSERT(profile, 0x3B7);
        AccomplishmentProgress &progress = profile->AccessAccomplishmentProgress();
        progress.ClearAllPerfectMoves();
        progress.ClearPerfectStreak();
    }
    mLastPlayedMode = TheGameMode->Mode();
    for (int i = 0; i < 2; i++) {
        ClearAndShrink(mMoveScores[i]);
        mMovesAttempted[i] = 0;
    }
    ClearAndShrink(mRecommendedPracticeMoves);
    mSkillsAwards->Clear();
    ClearAndShrink(unk74);
    if (TheGameMode->IsGameplayModePractice()) {
        SetUpRecapResults();
    }
}

void MetaPerformer::SetUpRecapResults() {
    static Symbol review("review");
    const std::vector<PracticeStep> &steps = GetPracticeSteps();
    FOREACH (it, steps) {
        if (it->mType == review) {
            std::vector<bool> bVec;
            int startBeat = Round(TheHamDirector->BeatFromTag(it->mStart));
            int endBeat = Round(TheHamDirector->BeatFromTag(it->mEnd));
            int diff = (endBeat - startBeat) / 4;
            for (int i = 0; i < diff; i++) {
                bVec.push_back(false);
            }
            unk74.push_back(bVec);
        }
    }
}

void MetaPerformer::PopulatePlaylistSongProvider(HamNavProvider *prov) const {
    if (!prov) {
        MILO_NOTIFY(
            "NULL PROVIDER PASSED INTO MetaPerformer::PopulatePlaylistSongProvider!!!"
        );
    } else {
        prov->Items().clear();
        int numSongs = mPlaylist->GetNumSongs();
        prov->Items().resize(numSongs);
        for (int i = 0; i < numSongs; i++) {
            DataArray *arr;
            const HamSongMetadata *data = TheHamSongMgr.Data(mPlaylist->GetSong(i));
            if (data) {
                const char *str = MakeString("%d. %s", i + 1, data->Title());
                const char *time = FormatTimeMS(mPlaylist->GetSongDuration(i));
                arr = new DataArray(2);
                arr->Node(0) = Symbol(str);
                arr->Node(1) = Symbol(time);
            } else {
                static Symbol song_unknown("song_unknown");
                const char *str = MakeString(
                    "%d. %s", i + 1, Localize(song_unknown, nullptr, TheLocale)
                );
                arr = new DataArray(2);
                arr->Node(0) = Symbol(str);
                arr->Node(1) = Symbol(" ");
            }
            prov->SetLabels(i, arr);
            arr->Release();
        }
    }
}

void MetaPerformer::OnReviewMovePassed(
    int playerIndex, HamMove *move, int ratingIndex, float f4
) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x455);
    HamPlayerData *pPlayerData = TheGameData->Player(playerIndex);
    MILO_ASSERT(pPlayerData, 0x458);
    if (mMoveScores[playerIndex].size() == 0) {
        MoveDir *moves = TheHamDirector->GetWorld()->Find<MoveDir>("moves");
        std::vector<HamMoveKey> keys;
        TheHamDirector->MoveKeys(pPlayerData->GetDifficulty(), moves, keys);
        mMoveScores[playerIndex].reserve(keys.size());
    }
    HamMoveScore score;
    score.unk8 = f4;
    score.unk0 = move;
    score.unk4 = ratingIndex;
    score.unkc = false;
    mMoveScores[playerIndex].push_back(score);
    static Symbol move_awesome("move_awesome");
    int awesomeIdx = RatingStateToIndex(move_awesome);
    int i90, i80;
    GetCurrentRecapMove(i90, i80);
    if (i90 >= 0 && i80 >= 0) {
        auto &set = unk74[i90][i80];
        if (ratingIndex <= awesomeIdx) {
            set = false;
        } else {
            set = true;
        }
    }
}

const std::vector<PracticeStep> &MetaPerformer::GetPracticeSteps() const {
    MoveDir *moves = TheHamDirector->GetWorld()->Find<MoveDir>("moves");
    MILO_ASSERT(moves, 0x7AF);
    PracticeSection *section = nullptr;
    for (ObjDirItr<PracticeSection> it(moves, true); it != nullptr; ++it) {
        if (it->GetDifficulty()
            == TheGameData->Player(TheHamProvider->Property("ui_nav_player")->Int())
                   ->GetDifficulty()) {
            section = it;
            break;
        }
    }
    MILO_ASSERT(section, 0x7BB);
    return section->Steps();
}

void MetaPerformer::OnRecallMovePassed(int playerIndex, HamMove *move) {
    MILO_ASSERT_RANGE(playerIndex, 0, 2, 0x443);
    FOREACH (it, mMoveScores[playerIndex]) {
        if (it->unk0 == move) {
            break;
        }
    }
    mMoveScores[playerIndex].clear();
}

void MetaPerformer::UpdateSongFromPlaylist() {
    int infinite = TheGameMode->Infinite();
    bool infiniteParty = TheHamProvider->Property("is_in_infinite_party_mode")->Int();
    if ((!infinite && !infiniteParty) || TheGameMode->InMode("campaign")) {
        MILO_ASSERT(mPlaylist, 0x6F5);
        int songID = mPlaylist->GetSong(mPlaylistIndex);
        Symbol song = TheHamSongMgr.GetShortNameFromSongID(songID);
        SelectSong(song, mPlaylistIndex);
    } else {
        Symbol song = TheHamSongMgr.GetRandomSong();
        TheHamSongMgr.GetSongIDFromShortName(song);
        SelectSong(song, mPlaylistIndex);
    }
}

void MetaPerformer::SaveDanceBattleScores(Symbol s1) {
    static Symbol score("score");
    int i6 = 0;
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayerData = TheGameData->Player(i);
        MILO_ASSERT(pPlayerData, 0x203);
        Hmx::Object *pPlayerProvider = pPlayerData->Provider();
        MILO_ASSERT(pPlayerProvider, 0x205);
        const DataNode *pScoreNode = pPlayerProvider->Property(score);
        MILO_ASSERT(pScoreNode, 0x207);
        if (pScoreNode->Int() > 0) {
            i6++;
        }
    }
    if (TheGameMode->IsGameplayModeDanceBattle() && i6 >= 2) {
        int winner = DetermineDanceBattleWinner();
        static Symbol p1("p1");
        static Symbol p2("p2");
        int songID = mSongMgr.GetSongIDFromShortName(s1);
        for (int i = 0; i < 2; i++) {
            HamPlayerData *pPlayerData = TheGameData->Player(i);
            MILO_ASSERT(pPlayerData, 0x21E);
            Hmx::Object *pPlayerProvider = pPlayerData->Provider();
            MILO_ASSERT(pPlayerProvider, 0x220);
            int padnum = pPlayerData->PadNum();
            HamProfile *profile = TheProfileMgr.GetProfileFromPad(padnum);
            if (profile && !TheAccomplishmentMgr->Unk30(padnum)) {
                SongStatusMgr *songStatusMgr = profile->GetSongStatusMgr();
                MILO_ASSERT(songStatusMgr, 0x229);
                static Symbol score("score");
                int scoreValue = pPlayerProvider->Property(score)->Int();
                if (scoreValue > 0) {
                    profile->UpdateBattleScore(
                        songID, pPlayerData, scoreValue, winner - i == 0
                    );
                }
            }
        }
    }
}

void MetaPerformer::CalcCharacters(
    const HamSongMetadata *data,
    bool,
    PlayerFlag flags,
    HamPlayerData *&primaryPlayer,
    Symbol &primaryCrew,
    Symbol &primaryChar,
    Symbol &primaryOutfit,
    HamPlayerData *&secondaryPlayer,
    Symbol &secondaryCrew,
    Symbol &secondaryChar,
    Symbol &secondaryOutfit
) {
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    Symbol s8c = pPlayer1Data->Unk48();
    Symbol s90 = pPlayer2Data->Unk48();
    if (flags == 0 || flags == 2) {
        s8c = gNullStr;
    }
    if (flags == 1 || flags == 2) {
        s90 = gNullStr;
    }
    bool has_s8c = s8c != gNullStr;
    bool has_s90 = s90 != gNullStr;
    if (has_s8c && has_s90 && !CharConflict(s8c, s90)) {
        primaryPlayer = pPlayer1Data;
        secondaryPlayer = pPlayer2Data;
        primaryCrew = GetCrewForCharacter(s8c);
        primaryChar = s8c;
        Symbol primaryPreferredOutfit = primaryPlayer->GetPreferredOutfit();
        primaryOutfit = GetUnlockedOutfit(primaryPreferredOutfit);
        secondaryCrew = GetCrewForCharacter(s90);
        secondaryChar = s90;
        Symbol secondaryPreferredOutfit = secondaryPlayer->GetPreferredOutfit();
        secondaryOutfit = GetUnlockedOutfit(secondaryPreferredOutfit);
    } else {
        int skeleton1 = pPlayer1Data->GetSkeletonTrackingID();
        int skeleton2 = pPlayer2Data->GetSkeletonTrackingID();

        CalcPrimarySongCharacter(data, primaryCrew, primaryChar, primaryOutfit);
        if (s90 != gNullStr || s8c != gNullStr) {
            if (s90 == gNullStr) {
                if (!CharConflict(s8c, primaryChar)) {
                    secondaryChar = primaryChar;
                }
            } else {
            }
        }
    }
}

void MetaPerformer::HandleGameplayEnded(const EndGameResult &egr) {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x377);
        HamProfile *pProfileFromPad = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
        Hmx::Object *pPlayerProvider = pPlayer->Provider();
        MILO_ASSERT(pPlayerProvider, 0x37e);

        static Symbol score("score");
        const DataNode *scoreNode = pPlayerProvider->Property(score);
        if (TheGameMode->Infinite() != 0 && 0 < scoreNode->Int()) {
            static Symbol cumulative_score("cumulative_score");
            const DataNode *cumulativeNode = pPlayerProvider->Property(cumulative_score);
            pPlayerProvider->SetProperty(
                cumulative_score, scoreNode->Int() + cumulativeNode->Int()
            );
        }

        if (pProfileFromPad) {
            if (pProfileFromPad->HasValidSaveData()) { // and something else
                if (0 < scoreNode->Int()) {
                    pProfileFromPad->GetMetagameStats()->HandleGameplayEnded(
                        pProfileFromPad, pPlayer, egr
                    );
                }
                if (TheGameMode->InMode("campaign") && egr == kEndGameResult_3) {
                    pProfileFromPad->DiscardRecentCampaignProgress();
                }
            }
        }
    }
    TheRockCentral.ManageJob(new GameEndedDataPointJob(this, egr));
}

void MetaPerformer::SaveAndUploadScores(Symbol s, int i1, int i2) {
    static Symbol score("score");
    int count = 0;
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayerData = TheGameData->Player(i);
        MILO_ASSERT(pPlayerData, 0x189);
        Hmx::Object *pPlayerProvider = pPlayerData->Provider();
        MILO_ASSERT(pPlayerProvider, 0x18c);
        const DataNode *pScoreNode = pPlayerProvider->Property(score);
        MILO_ASSERT(pScoreNode, 0x18f);
        if (0 < pScoreNode->Int()) {
            count++;
        }
    }

    if (0 < count) {
        // something here with i1
        static Symbol p1("p1");
        static Symbol p2("p2");
        static Symbol alert_highscore_solo("alert_highscore_solo");
        static Symbol alert_highscore_coop("alert_highscore_coop");

        for (int i = 0; i < 2; i++) {
            HamPlayerData *pPlayerData = TheGameData->Player(i);
            MILO_ASSERT(pPlayerData, 0x1ad);
            Hmx::Object *pPlayerProvider = pPlayerData->Provider();
            MILO_ASSERT(pPlayerProvider, 0x1af);
        }
    }
}

void MetaPerformer::CalculatePracticeResults() {
    mNumLearnMovesPassed = 0;
    mNumLearnMovesFastLaned = 0;
    mNumLearnMovesTotal = 0;
    mPracticeLearnScore = 0;
    mNumReviewMovesPassed = 0;
    mNumReviewMovesTotal = 0;
    mPracticeReviewScore = 0;
    MoveDir *moves = TheHamDirector->GetWorld()->Find<MoveDir>("moves");
    MILO_ASSERT(moves, 0x4a4);
    ObjDirItr<PracticeSection> sections(moves, true);
}

void MetaPerformer::SetDefaultSongCharacter(int i) {
    Symbol primaryCrew;
    Symbol primaryChar;
    Symbol primaryOutfit;
    HamPlayerData *primaryPlayer;
    Symbol secondaryCrew;
    Symbol secondaryChar;
    Symbol secondaryOutfit;
    HamPlayerData *pSecondary;

    const HamSongMetadata *pSongData =
        TheHamSongMgr.Data(TheHamSongMgr.GetSongIDFromShortName(TheGameData->GetSong()));
    MILO_ASSERT(pSongData, 0x592);
    bool modeCheck =
        TheGameMode->InMode("dance_battle") || TheGameMode->InMode("strike_a_pose");
    CalcCharacters(
        pSongData,
        modeCheck,
        (PlayerFlag)i,
        primaryPlayer,
        primaryCrew,
        primaryChar,
        primaryOutfit,
        pSecondary,
        secondaryCrew,
        secondaryChar,
        secondaryOutfit
    );
    HamPlayerData *pPlayerData = TheGameData->Player(i);
    if (pPlayerData == primaryPlayer) {
        primaryPlayer->SetCharacter(primaryChar);
        primaryPlayer->SetOutfit(primaryOutfit);
        primaryPlayer->SetCrew(primaryCrew);
        if (primaryPlayer->Char() == pSecondary->Char()) {
            pSecondary->SetCharacter(secondaryChar);
            pSecondary->SetOutfit(secondaryOutfit);
            pSecondary->SetCrew(secondaryCrew);
        }
    } else {
        MILO_ASSERT(pPlayerData == pSecondary, 0x5a8);
        pSecondary->SetCharacter(secondaryChar);
        pSecondary->SetOutfit(secondaryOutfit);
        pSecondary->SetCrew(secondaryCrew);
        if (primaryPlayer->Char() == pSecondary->Char()) {
            primaryPlayer->SetCharacter(primaryChar);
            primaryPlayer->SetOutfit(primaryOutfit);
            primaryPlayer->SetCrew(primaryCrew);
        }
    }
}

bool MetaPerformer::CheckRecommendedPracticeMove(String s, int player) const {
    MILO_ASSERT(player>=0 && player < MULTIPLAYER_SLOTS, 0x4eb);
    int val1 = 0;
    int val2 = 0;
    bool check = false;
    for (int i = 0; i < mMoveScores[player].size(); i++) {
        int rating = mMoveScores[player][i].unk4;
        if (s == mMoveScores[player][i].unk0->DisplayName()) {
            val1++;
            check = false;
            if (2 < rating) {
                val2++;
                check = true;
            }
        }
    }
    if (!check && (float)val2 / (float)val1 <= 0.49f) {
        return false;
    }
}

void MetaPerformer::GetCurrentRecapMove(int &i1, int &i2) const {
    int x = 0;
    static Symbol review("review");
    auto &steps = GetPracticeSteps();
    FOREACH (it, steps) {
        if (it->mType == review) {
            std::vector<bool> vec;
            int start = Round(TheHamDirector->BeatFromTag(it->mStart));
            int end = Round(TheHamDirector->BeatFromTag(it->mEnd));
            int beat = Round(TheTaskMgr.Beat());
            if (start <= beat && end >= beat) {
                i1 = x;
                i2 = ((beat - start) / 4) - 1;
                return;
            }
            x++;
        }
    }
    MILO_NOTIFY("Couldn\'t find a recap move at beat %f", TheTaskMgr.Beat());
    i1 = -1;
    i2 = -1;
}

#pragma endregion
#pragma region MetaPerformerHook

MetaPerformerHook::MetaPerformerHook(const HamSongMgr &mgr)
    : mQuickplayPerformer(new QuickplayPerformer(mgr)),
      mCampaignPerformer(new CampaignPerformer(mgr)) {
    SetName("meta_performer", ObjectDir::Main());
}

MetaPerformerHook::~MetaPerformerHook() { delete mQuickplayPerformer; }

BEGIN_HANDLERS(MetaPerformerHook)
    HANDLE_EXPR(current, Current())
    HANDLE_MEMBER_PTR(Current())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

MetaPerformer *MetaPerformerHook::Current() {
    if (TheGameMode->InMode("campaign")) {
        return mCampaignPerformer;
    } else {
        return mQuickplayPerformer;
    }
}
