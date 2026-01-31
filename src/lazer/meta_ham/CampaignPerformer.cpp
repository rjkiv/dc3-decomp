#include "meta_ham/CampaignPerformer.h"
#include "SaveLoadManager.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPlayerData.h"
#include "meta/SongMgr.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/Campaign.h"
#include "meta_ham/CampaignEra.h"
#include "meta_ham/CampaignProgress.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SongStatusMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

CampaignPerformer::CampaignPerformer(const HamSongMgr &hsm)
    : MetaPerformer(hsm, "campaign_performer"), mEra(gNullStr),
      mDifficulty(kDifficultyEasy), mJustFinishedEra(false), mJustUnlockedEraSong(false),
      mWasLastMoveMastered(false), mLastEraStars(-1), mLastEraMoves(-1),
      mStarsEarnedSoFar(0) {}

BEGIN_HANDLERS(CampaignPerformer)
    HANDLE_EXPR(is_campaign_new, IsCampaignNew())
    HANDLE_EXPR(is_campaign_intro_complete, IsCampaignIntroComplete())
    HANDLE_ACTION(set_campaign_intro_complete, SetCampaignIntroComplete(true))
    HANDLE_EXPR(is_campaign_mind_control_complete, IsCampaignMindControlComplete())
    HANDLE_ACTION(set_campaign_mind_control_complete, SetCampaignMindControlComplete(true))
    HANDLE_EXPR(is_campaign_complete, IsCampaignComplete())
    HANDLE_ACTION(set_campaign_complete, SetCampaignComplete())
    HANDLE_ACTION(setup_campaign_intro_playlist, SetIntroPlaylist())
    HANDLE_ACTION(setup_campaign_outro_playlist, SetOutroPlaylist())
    HANDLE_EXPR(get_era, mEra)
    HANDLE_ACTION(set_era, SetEra(_msg->Sym(2)))
    HANDLE_EXPR(first_era, GetFirstEra())
    HANDLE_EXPR(last_era, GetLastEra())
    HANDLE_EXPR(tan_battle_era, GetTanBattleEra())
    HANDLE_EXPR(just_finished_era, mJustFinishedEra)
    HANDLE_EXPR(set_era_to_first_incomplete, SetEraToFirstIncomplete())
    HANDLE_EXPR(is_era_new, IsEraNew())
    HANDLE_EXPR(is_current_era_complete, IsEraComplete(mEra))
    HANDLE_EXPR(is_era_complete, IsEraComplete(_msg->Sym(2)))
    HANDLE_ACTION(set_song, MetaPerformer::SetSong(_msg->Sym(2)))
    HANDLE_EXPR(num_era_songs, GetNumEraSongs())
    HANDLE_EXPR(get_era_song, GetEraSong(_msg->Int(2)))
    HANDLE_EXPR(get_era_intro_song, GetEraIntroSong())
    HANDLE_EXPR(get_song_attempted_count, GetSongAttemptedCount())
    HANDLE_EXPR(has_song_been_attempted, HasSongBeenAttempted(_msg->Sym(2)))
    HANDLE_EXPR(get_song_index, GetSongIndex(_msg->Sym(2)))
    HANDLE_EXPR(get_num_song_craze_moves, GetNumSongCrazeMoves(_msg->Sym(2)))
    HANDLE_EXPR(can_select_era_song, CanSelectEraSong(_msg->Sym(2)))
    HANDLE_EXPR(is_era_move_mastered, IsEraMoveMastered(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(just_unlocked_erasong, mJustUnlockedEraSong)
    HANDLE_ACTION(clear_just_unlocked_erasong, mJustUnlockedEraSong = false)
    HANDLE_ACTION(clear_last_era_stars, mLastEraStars = -1)
    HANDLE_ACTION(clear_last_era_moves, mLastEraMoves = -1)
    HANDLE_EXPR(get_last_era_stars, mLastEraStars)
    HANDLE_EXPR(is_dance_craze_song_available, IsDanceCrazeSongAvailable(_msg->Sym(2)))
    HANDLE_EXPR(is_era_mastered, IsEraMastered(_msg->Sym(2)))
    HANDLE_EXPR(has_era_outfits, HasEraOutfits(_msg->Sym(2)))
    HANDLE_EXPR(get_dance_craze_song, GetDanceCrazeSong())
    HANDLE_EXPR(is_attempting_dance_craze_song, IsAttemptingDanceCrazeSong())
    HANDLE_EXPR(get_erasong_unlocked_token, GetEraSongUnlockedToken())
    HANDLE_EXPR(get_era_complete_token, GetEraCompleteToken())
    HANDLE_EXPR(get_win_instructions_token, GetWinInstructionsToken())
    HANDLE_EXPR(get_era_intro_movie, GetEraIntroMovieToken())
    HANDLE_EXPR(get_era_intro_movie_played, GetEraIntroMoviePlayed())
    HANDLE_ACTION(set_era_intro_movie_played, SetEraIntroMoviePlayed(true))
    HANDLE_EXPR(get_era_stars_earned, GetEraStarsEarned(_msg->Sym(2)))
    HANDLE_EXPR(get_song_stars_earned, GetSongStarsEarned(_msg->Sym(2), _msg->Sym(3)))
    HANDLE_EXPR(get_required_mastery_stars, GetStarsRequiredForMastery(_msg->Sym(2)))
    HANDLE_EXPR(get_required_outfit_stars, GetStarsRequiredForOutfits(_msg->Sym(2)))
    HANDLE_EXPR(get_mastery_moves, GetMasteryMoves(_msg->Sym(2)))
    HANDLE_EXPR(get_required_mastery_moves, GetMovesRequiredForMastery(_msg->Sym(2)))
    HANDLE_ACTION(award_craze_accomplishments, AwardCrazeAccomplishments())
    HANDLE_EXPR(get_completion_accomplishment, GetCompletionAccomplishment(_msg->Sym(2)))
    HANDLE_ACTION(award_boss_accomplishment, AwardBossAccomplishment())
    HANDLE_ACTION(award_master_quest_accomplishment, AwardMasterQuestAccomplishments())
    HANDLE_EXPR(
        is_dance_craze_move,
        IsDanceCrazeMove(_msg->Sym(2), _msg->Sym(3), _msg->Obj<HamMove>(4))
    )
    HANDLE_EXPR(
        is_dance_craze_move_mastered,
        IsDanceCrazeMoveMastered(_msg->Sym(2), _msg->Sym(3), _msg->Obj<HamMove>(4))
    )
    HANDLE_ACTION(restart_finale_song, mNumCompleted.pop_back())
    HANDLE_EXPR(get_challenge_character, GetChallengeCharacter())
    HANDLE_EXPR(was_last_move_mastered, mWasLastMoveMastered)
    HANDLE_EXPR(last_move_mastered_name, mLastMoveMasteredName)
    HANDLE_ACTION(set_stars_earned, UpdateStarsEarnedSoFar(_msg->Int(2)))
    HANDLE_ACTION(clear_all_campaign_progress, ClearAllCampaignProgress())
    HANDLE_ACTION(on_load_song, OnLoadSong())
    HANDLE_EXPR(won_current_outro_song, WonCurrentOutroSong())
    HANDLE_EXPR(in_outro_perform, InOutroPerform())
    HANDLE_ACTION(bookmark_current_progress, BookmarkCurrentProgress())
    HANDLE_ACTION(
        unlock_all_moves, UnlockAllMoves(_msg->Sym(2), _msg->Sym(3), _msg->Int(4))
    )
    HANDLE_ACTION(reset_all_campaign_progress, ResetAllCampaignProgress())
    HANDLE_SUPERCLASS(MetaPerformer)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CampaignPerformer)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

void CampaignPerformer::SelectSong(Symbol song, int i) {
    TheGameData->SetSong(song);
    mJustUnlockedEraSong = false;
    mStarsEarnedSoFar = 0;
    if (TheGameMode->InMode("campaign_perform", true)) {
        BookmarkCurrentProgress();
        CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
        MILO_ASSERT(pEra, 0x58);
        Symbol crew = pEra->Crew();
        CampaignEraSongEntry *pSongEntry = pEra->GetSongEntry(song);
        MILO_ASSERT(pSongEntry, 0x5c);
        Symbol introCrew = pSongEntry->GetUnk8();
        SetupCampaignCharacters(crew, introCrew);
    } else if (TheGameMode->InMode("campaign_intro", true)) {
        static Symbol era_tan_battle("era_tan_battle");
        if (mEra == era_tan_battle)
            return;
        Symbol crew = TheCampaign->GetIntroCrew();
        HamPlayerData *pPlayer1Data = TheGameData->Player(0);
        MILO_ASSERT(pPlayer1Data, 0x68);
        HamPlayerData *pPlayer2Data = TheGameData->Player(1);
        MILO_ASSERT(pPlayer2Data, 0x6a);
        Symbol introCrew = TheCampaign->GetIntroSongCharacter(i);
        SetupCampaignCharacters(crew, introCrew);
    }
}

void CampaignPerformer::CompleteSong(int i1, int i2, int i3, float f4, bool b5) {
    Symbol song = TheGameData->GetSong();
    if (TheGameMode->InMode("campaign_perform", true)
        || TheGameMode->InMode("campaign_holla_back", true)) {
        static Symbol gameplay_mode("gameplay_mode");
        Symbol mode = TheGameMode->Property(gameplay_mode)->Sym();
        static Symbol perform("perform");
        if (mode == perform) {
            UpdateEraSong(mDifficulty, mEra, song, i1);
        }
    } else if (TheGameMode->InMode("campaign_intro", true)
               && !TheGameMode->InMode("campaign_outro", true)) {
        int stars = TheCampaign->GetIntroSongStarsRequired(GetPlaylistIndex());
        if (IsLastSong() && i1 >= stars) {
            SetCampaignIntroComplete(true);
        }
    } else if (TheGameMode->InMode("campaign_outro", true)) {
        int stars = TheCampaign->GetOutroSongStarsRequired(GetPlaylistIndex());
        TheHamProvider->SetProperty("merge_moves", 0);
    }
    MetaPerformer::CompleteSong(i1, i2, i3, f4, b5);
    BookmarkCurrentProgress();
}

void CampaignPerformer::OnLoadSong() {
    mStarsEarnedSoFar = 0;
    if (TheGameMode->InMode("campaign_outro", true)) {
        int idx = GetPlaylistIndex();
        static Symbol perform("perform");
        static Symbol song_shortening_enabled("song_shortening_enabled");
        static Symbol deinit("deinit");
        Symbol mode = TheCampaign->GetOutroSongGameplayMode(idx);
        if (TheGameMode->GameplayMode() != mode) {
            TheGameMode->SetGameplayMode(mode, mode == perform);
            TheHamProvider->SetProperty(
                song_shortening_enabled,
                TheCampaign->GetOutroSongShortened(GetPlaylistIndex())
            );
        }
    }
}

void CampaignPerformer::OnMovePassed(int player, HamMove *move, int i3, float f4) {
    MILO_ASSERT(TheGameMode->InMode( "campaign" ), 0x2C7);
    MetaPerformer::OnMovePassed(player, move, i3, f4);
    if (!TheGameMode->InMode("campaign_intro")) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x2D2);
        HamPlayerData *hpd = TheGameData->Player(player);
        HamProfile *pProfileFromPad = TheProfileMgr.GetProfileFromPad(hpd->PadNum());
        if (pProfileFromPad != pProfile) {
            MILO_LOG(
                "CampaignPerformer::OnMovePassed: Campaign progress earned by Player '%s' is credited to Player '%s'\n",
                pProfileFromPad ? pProfileFromPad->GetName() : "NOTSIGNEDIN",
                pProfile->GetName()
            );
        }
        int allow_no_profile_in_campaign =
            DataVariable("allow_no_profile_in_campaign").Int();
        if (!pProfile) {
            MILO_ASSERT(pProfile || allow_no_profile_in_campaign, 0x2E0);
        } else {
            for (int i = 0; i <= mDifficulty; i++) {
                CampaignProgress &progress =
                    pProfile->AccessCampaignProgress((Difficulty)i);
                progress.SetSongPlayed(mEra, GetSong(), true);
            }
        }
        mWasLastMoveMastered = false;
        static Symbol gameplay_mode("gameplay_mode");
        static Symbol perform("perform");
        Symbol gamemode = TheGameMode->Property(gameplay_mode)->Sym();
        if (gamemode == perform) {
            Symbol tan("era_tan_battle");
            if (mEra != tan) {
                CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
                MILO_ASSERT(pEra, 0x2F6);
                bool b9 = false;
                for (int i = 0; i < pEra->GetNumSongs(); i++) {
                    Symbol songname = pEra->GetSongName(i);
                    if (GetSong() == songname) {
                        b9 = true;
                        break;
                    }
                }
                if (b9) {
                    Symbol moveVariantName =
                        pEra->GetMoveVariantName(GetSong(), move->Name());
                    if (pEra->HasCrazeMove(GetSong(), move->Name()) && i3 <= 1) {
                        static bool sPassed = true;
                        for (int i = 0; i <= mDifficulty; i++) {
                            if (sPassed && pProfile) {
                                CampaignProgress &progress =
                                    pProfile->AccessCampaignProgress((Difficulty)i);
                                if (progress.UpdateEraSongMoveMastered(
                                        mEra, GetSong(), move->Name()
                                    )
                                    && i == mDifficulty) {
                                    mWasLastMoveMastered = true;
                                    mLastMoveMasteredName = moveVariantName;
                                    CheckForEraSongUnlock();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool CampaignPerformer::InOutroPerform() const {
    return TheGameMode->InMode("campaign_outro", true)
        && TheCampaign->GetOutroSongFreestyleEnabled(GetPlaylistIndex());
}

void CampaignPerformer::SetDifficulty(Difficulty d) {
    mDifficulty = d;
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1Data, 0x149);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2Data, 0x14b);
    pPlayer1Data->SetDifficulty(d);
    pPlayer2Data->SetDifficulty(d);
}

int CampaignPerformer::GetSongStarsEarned(Symbol s1, Symbol s2) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x1e5);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.GetSongStarsEarned(s1, s2);
}

bool CampaignPerformer::IsEraNew() const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x32f);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return !pCampaignProgress.IsEraPlayed(mEra);
}

bool CampaignPerformer::IsCampaignNew() const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x337);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.IsCampaignNew();
}

bool CampaignPerformer::IsCampaignIntroComplete() const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x33f);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.IsCampaignIntroCompleted();
}

int CampaignPerformer::GetStarsRequiredForOutfits(Symbol era) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x1d1);
    return pEra->StarsRequiredForOutfits();
}

int CampaignPerformer::GetStarsRequiredForMastery(Symbol era) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x1ed);
    return pEra->StarsRequiredForMastery();
}

int CampaignPerformer::GetMovesRequiredForMastery(Symbol era) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x203);
    return pEra->MovesRequiredForMastery();
}

Symbol CampaignPerformer::GetCompletionAccomplishment(Symbol era) const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x248);
    return pEra->CompletionAccomplishment();
}

void CampaignPerformer::SetCampaignIntroComplete(bool completed) {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x347);
    for (int i = 0; i <= mDifficulty; i++) {
        CampaignProgress &pCampaignProgress =
            pProfile->AccessCampaignProgress((Difficulty)i);
        pCampaignProgress.SetCampaignIntroCompleted(completed);
    }
}

bool CampaignPerformer::IsCampaignMindControlComplete() const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x352);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.IsCampaignMindControlCompleted();
}

void CampaignPerformer::SetCampaignMindControlComplete(bool completed) {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x35a);
    for (int i = 0; i <= mDifficulty; i++) {
        CampaignProgress &pCampaignProgress =
            pProfile->AccessCampaignProgress((Difficulty)i);
        pCampaignProgress.SetCampaignMindControlCompleted(completed);
    }
}

bool CampaignPerformer::IsCampaignComplete() const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x365);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.IsCampaignTanBattleCompleted();
}

void CampaignPerformer::SetCampaignComplete() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x36d);
    for (int i = 0; i <= mDifficulty; i++) {
        CampaignProgress &pCampaignProgress =
            pProfile->AccessCampaignProgress((Difficulty)i);
        pCampaignProgress.SetCampaignTanBattleCompleted(true);
    }
}

bool CampaignPerformer::IsEraMastered(Symbol era) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x378);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.IsEraMastered(era);
}

bool CampaignPerformer::IsDanceCrazeSongAvailable(Symbol era) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x380);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.IsDanceCrazeSongAvailable(era);
}

bool CampaignPerformer::IsEraComplete(Symbol era) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x388);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.IsEraComplete(era);
}

bool CampaignPerformer::HasEraOutfits(Symbol era) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x390);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x395);
    if (pEra->OutfitAward() == gNullStr
        || pCampaignProgress.GetEraStarsEarned(era) >= pEra->StarsRequiredForOutfits()) {
        return true;
    } else {
        return false;
    }
}

Symbol CampaignPerformer::GetDanceCrazeSong() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x3a9);
    return pEra->GetDanceCrazeSong();
}

bool CampaignPerformer::IsAttemptingDanceCrazeSong() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x3b0);
    Symbol dcs = pEra->GetDanceCrazeSong();
    return TheGameData->GetSong() == dcs;
}

Symbol CampaignPerformer::GetEraSongUnlockedToken() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x3b9);
    return pEra->EraSongUnlockedToken();
}

Symbol CampaignPerformer::GetEraCompleteToken() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x3c0);
    return pEra->EraSongCompleteToken();
}

Symbol CampaignPerformer::GetWinInstructionsToken() const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x3c7);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);

    return TheCampaign->GetCampaignWinInstructions(
        pCampaignProgress.GetNumCompletedEras()
    );
}

Symbol CampaignPerformer::GetCharacterForSong() const {
    Symbol song = MetaPerformer::GetSong();
    int songID = TheHamSongMgr.GetSongIDFromShortName(song);
    const HamSongMetadata *pSongData = TheHamSongMgr.Data(songID);
    MILO_ASSERT(pSongData, 0x3d6);
    return pSongData->Character();
}

Symbol CampaignPerformer::GetEraIntroMovieToken() const {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x3ed);
    return pEra->GetIntroMovie();
}

Symbol CampaignPerformer::GetEraIntroSong() {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x44e);
    return pEra->GetSongName(Max(pEra->GetNumSongs() - 1, 0));
}

bool CampaignPerformer::IsEraMoveMastered(Symbol s, int i) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x4db);
    CampaignEraSongEntry *pSongEntry = pEra->GetSongEntry(s);
    MILO_ASSERT(pSongEntry, 0x4dd);
    Symbol crazeMoveHamMoveName = pSongEntry->GetCrazeMoveHamMoveName(i);
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x4e1);
    const CampaignProgress &pProgress = pProfile->GetCampaignProgress(mDifficulty);
    return pProgress.IsMoveMastered(pEra->GetName(), s, crazeMoveHamMoveName);
}

bool CampaignPerformer::GetEraIntroMoviePlayed() const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x3f5);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    return pCampaignProgress.GetEraIntroMoviePlayed(mEra);
}

void CampaignPerformer::SetEraIntroMoviePlayed(bool completed) {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x3fd);
    for (int i = 0; i <= mDifficulty; i++) {
        CampaignProgress &pCampaignProgress =
            pProfile->AccessCampaignProgress((Difficulty)i);
        pCampaignProgress.SetEraIntroMoviePlayed(mEra, completed);
    }
}

void CampaignPerformer::ClearAllCampaignProgress() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x4fa);
    if (pProfile) {
        for (int i = 0; i <= 2; i++) {
            CampaignProgress &pCampaignProgress =
                pProfile->AccessCampaignProgress((Difficulty)i);
            pCampaignProgress.ResetAllProgress();
            pCampaignProgress.BookmarkCurrentProgress();
        }
        if (TheSaveLoadMgr)
            TheSaveLoadMgr->AutoSave();
    }
}

Symbol CampaignPerformer::GetChallengeCharacter() const {
    Symbol song = MetaPerformer::GetSong();
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x3e0);
    if (pEra->GetDanceCrazeSong() == song)
        return gNullStr;
    else
        return GetCharacterForSong();
}

int CampaignPerformer::GetNumEraSongs() {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x43e);
    return pEra->GetNumSongs();
}

Symbol CampaignPerformer::GetEraSong(int i_iIndex) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x445);
    MILO_ASSERT(i_iIndex < pEra->GetNumSongs(), 0x446);
    return pEra->GetSongName(i_iIndex);
}

int CampaignPerformer::GetSongIndex(Symbol s) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x46b);
    return pEra->GetSongIndex(s);
}

int CampaignPerformer::GetNumSongCrazeMoves(Symbol s) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x473);
    return pEra->GetNumSongCrazeMoves(s);
}

void CampaignPerformer::BookmarkCurrentProgress() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x4e9);
    for (int i = 0; i <= mDifficulty; i++) {
        pProfile->AccessCampaignProgress((Difficulty)i).BookmarkCurrentProgress();
    }
}

void CampaignPerformer::ResetAllCampaignProgress() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x553);
    CampaignProgress &pCampaignProgress = pProfile->AccessCampaignProgress(mDifficulty);
    pCampaignProgress.ResetAllProgress();
    if (TheSaveLoadMgr)
        TheSaveLoadMgr->AutoSave();
}

void CampaignPerformer::ClearSongProgress(Symbol s1, Symbol s2) {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x55f);
    CampaignProgress &pCampaignProgress = pProfile->AccessCampaignProgress(mDifficulty);
    pCampaignProgress.ClearSongProgress(s1, s2);
}

void CampaignPerformer::UpdateStarsEarnedSoFar(int stars) {
    mStarsEarnedSoFar = stars;
    if (TheGameMode->InMode("campaign_outro", true)) {
        TheCampaign->SetOutroStarsEarned(MetaPerformer::GetPlaylistIndex(), stars);
    } else if (!IsEraMastered(mEra)) {
        CheckForEraSongUnlock();
    }
}

void CampaignPerformer::SetOutroPlaylist() {
    Symbol throneroom("throneroom");
    TheGameData->SetVenue(throneroom);
    Symbol tan("tan");
    Symbol oblio("oblio");
    Symbol tan04("tan04");
    Symbol oblio06("oblio06");
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1Data, 0x114);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2Data, 0x116);
    pPlayer1Data->SetCharacter(tan);
    pPlayer2Data->SetCharacter(oblio);
    pPlayer1Data->SetOutfit(tan04);
    pPlayer2Data->SetOutfit(oblio06);
    static Symbol is_in_infinite_party_mode("is_in_infinite_party_mode");
    TheHamProvider->SetProperty(is_in_infinite_party_mode, false);
    mOutroPlaylist.Clear();
    int numOutroSongs = TheCampaign->GetNumOutroSongs();
    for (int i = 0; i < numOutroSongs; i++) {
        Symbol outroSong = TheCampaign->GetOutroSong(i);
        int songID = TheHamSongMgr.GetSongIDFromShortName(outroSong);
        mOutroPlaylist.AddSong(songID);
    }
    MetaPerformer::SetPlaylist(&mOutroPlaylist);
}

void CampaignPerformer::SetIntroPlaylist() {
    Symbol introVenue = TheCampaign->GetIntroVenue();
    TheGameData->SetVenue(introVenue);
    Symbol introCrew = TheCampaign->GetIntroCrew();
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1Data, 0xe5);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2Data, 0xe7);
    pPlayer1Data->SetCrew(introCrew);
    pPlayer2Data->SetCrew(introCrew);
    Symbol introSongCharacter = TheCampaign->GetIntroSongCharacter(0);
    SetupCampaignCharacters(introCrew, introSongCharacter);
    static Symbol is_in_infinite_party_mode("is_in_infinite_party_mode");
    TheHamProvider->SetProperty(is_in_infinite_party_mode, false);
    mIntroPlaylist.Clear();
    int numIntroSongs = TheCampaign->GetNumIntroSongs();
    for (int i = 0; i < numIntroSongs; i++) {
        Symbol introSong = TheCampaign->GetIntroSong(i);
        int songID = TheHamSongMgr.GetSongIDFromShortName(introSong);
        mIntroPlaylist.AddSong(songID);
    }
    MetaPerformer::SetPlaylist(&mIntroPlaylist);
    mJustFinishedEra = false;
    mJustUnlockedEraSong = false;
    mStarsEarnedSoFar = 0;
    mLastEraStars = -1;
    mLastEraMoves = -1;
}

int CampaignPerformer::GetEraStarsEarned(Symbol era) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x1d9);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    int eraStarsEarned = pCampaignProgress.GetEraStarsEarned(era);
    int starsForMastery = GetStarsRequiredForMastery(era);

    if (eraStarsEarned >= starsForMastery)
        eraStarsEarned = starsForMastery;
    return eraStarsEarned;
}

Symbol CampaignPerformer::GetFirstEra() const {
    CampaignEra *pEra = TheCampaign->GetFirstEra();
    MILO_ASSERT(pEra, 0x32);
    return pEra->GetName();
}

int CampaignPerformer::GetMasteryMoves(Symbol era) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x1f6);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    int eraMovesEarned = pCampaignProgress.GetEraMovesMastered(era);
    int movesForMastery = GetMovesRequiredForMastery(era);

    if (eraMovesEarned >= movesForMastery)
        eraMovesEarned = movesForMastery;
    return eraMovesEarned;
}

bool CampaignPerformer::SetEraToFirstIncomplete() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x136);
    const CampaignProgress &pCampaignProgress =
        pProfile->GetCampaignProgress(mDifficulty);
    Symbol firstIncompleteEra = pCampaignProgress.GetFirstIncompleteEra();
    if (firstIncompleteEra != mEra) {
        SetEra(firstIncompleteEra);
        return true;
    }
    return false;
}

void CampaignPerformer::SetEra(Symbol era) {
    mEra = era;
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0xba);
    TheHamProvider->SetProperty("current_campaign_era", mEra);
    Symbol crew = pEra->Crew();
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1Data, 0xc2);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2Data, 0xc4);
    pPlayer1Data->SetCrew(crew);
    pPlayer2Data->SetCrew(crew);
    SetupCampaignCharacters(crew, gNullStr);
    TheGameData->SetVenue(pEra->Venue());
    mJustFinishedEra = false;
    mJustUnlockedEraSong = false;
    mStarsEarnedSoFar = 0;
    mLastEraStars = -1;
    mLastEraMoves = -1;
    TheCampaign->LoadCampaignDanceMoves(mEra);
}

void CampaignPerformer::CheckForMasteryGoal(Difficulty diff, Symbol era) {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x1bd);
    const CampaignProgress &pProgress = pProfile->GetCampaignProgress(diff);
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x1c0);
    Symbol masteryStars = pEra->GetMasteryStars(diff);

    if (masteryStars != gNullStr) {
        int eraEarned = pProgress.GetEraStarsEarned(era);
        if (eraEarned >= pEra->StarsRequiredForMastery()) {
            TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                pProfile, masteryStars, true
            );
        }
    }
}

void CampaignPerformer::CheckForOutfitAwards(Difficulty diff, Symbol era) {
    HamProfile *pActiveProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pActiveProfile, 0x17d);
    const CampaignProgress &pProgress = pActiveProfile->GetCampaignProgress(diff);
    CampaignEra *pEra = TheCampaign->GetCampaignEra(era);
    MILO_ASSERT(pEra, 0x180);
    Symbol outfitAwards = pEra->OutfitAward();

    if (outfitAwards != gNullStr) {
        int eraEarned = pProgress.GetEraStarsEarned(era);
        if (pProgress.IsEraComplete(era)
            && eraEarned >= pEra->StarsRequiredForOutfits()) {
            TheAccomplishmentMgr->EarnAwardForAll(outfitAwards, false);
        }
    }
}

bool CampaignPerformer::CanSelectEraSong(Symbol song) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x4bc);
    CampaignEraSongEntry *pSongEntry = pEra->GetSongEntry(song);
    MILO_ASSERT(pSongEntry, 0x4be);
    Symbol introSong = GetEraIntroSong();
    if (!HasSongBeenAttempted(introSong)) {
        return song == introSong;
    } else {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x4d3);
        const CampaignProgress &pProgress = pProfile->GetCampaignProgress(mDifficulty);
        return pProgress.IsEraSongAvailable(mEra, song);
    }
}

void CampaignPerformer::AwardBossAccomplishment() {
    if (IsCampaignComplete()) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x223);
        static Symbol acc_boss("acc_boss");
        TheAccomplishmentMgr->EarnAccomplishmentForProfile(pProfile, acc_boss, false);
    }
}

bool CampaignPerformer::WonCurrentOutroSong() const {
    int idx = GetPlaylistIndex();
    int starsEarned = TheCampaign->GetOutroStarsEarned(idx);
    int starsRequired = TheCampaign->GetOutroSongStarsRequired(idx);
    if (!InOutroPerform()) {
        return starsRequired <= starsEarned;
    } else {
        static Symbol freestyle_enabled("freestyle_enabled");
        return TheHamDirector->Property(freestyle_enabled)->Int() || starsEarned >= 5;
    }
}

void CampaignPerformer::CheckForEraSongUnlock() {
    if (!mJustUnlockedEraSong) {
        Symbol song = TheGameData->GetSong();
        CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
        MILO_ASSERT(pEra, 0x271);
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x273);
        const CampaignProgress &progress = pProfile->GetCampaignProgress(mDifficulty);
        int eraStarsEarned = progress.GetEraStarsEarned(mEra);
        int songStarsEarned = progress.GetSongStarsEarned(mEra, song);
        int starsForMastery = pEra->StarsRequiredForMastery();
        int totalStarsEarned = (mStarsEarnedSoFar - songStarsEarned) + eraStarsEarned;
        int movesMastered = progress.GetEraMovesMastered(mEra);
        int movesForMastery = pEra->MovesRequiredForMastery();
        if (totalStarsEarned >= starsForMastery && movesMastered >= movesForMastery) {
            mJustUnlockedEraSong = true;
        }
        MILO_LOG(
            "==[CampaignPerformer::CheckForEraSongUnlock]== song=%s, stars this song=%d : totals = %d/%d stars, %d/%d moves  %s\n",
            song.Str(),
            mStarsEarnedSoFar,
            totalStarsEarned,
            starsForMastery,
            movesMastered,
            movesForMastery,
            mJustUnlockedEraSong ? "***JustUnlockedEraSong***" : ""
        );
    }
}

bool CampaignPerformer::HasSongBeenAttempted(Symbol song) {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    if (!pEra) {
        MILO_NOTIFY(
            "CampaignPerformer::HasSongBeenAttempted, returning false because no era found for '%s'\n",
            mEra.Str()
        );
        return false;
    } else {
        CampaignEraSongEntry *pEntry = pEra->GetSongEntry(song);
        if (!pEntry) {
            MILO_NOTIFY(
                "CampaignPerformer::HasSongBeenAttempted, returning false because no song entry found for '%s'\n",
                song.Str()
            );
            return false;
        } else {
            HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
            MILO_ASSERT(pProfile, 0x488);
            const CampaignProgress &progress = pProfile->GetCampaignProgress(mDifficulty);
            return progress.IsSongPlayed(mEra, song);
        }
    }
}

void CampaignPerformer::UnlockAllMoves(Symbol s1, Symbol s2, int i3) {
    if (TheGameMode->InMode("campaign") && !TheCampaign->InDCICutscene()) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x53C);
        SongStatusMgr *pSongStatusMgr = pProfile->GetSongStatusMgr();
        MILO_ASSERT(pSongStatusMgr, 0x53F);
        int songID = TheSongMgr.GetSongIDFromShortName(s2);
        pSongStatusMgr->UpdateSong(
            songID, 0x29A, 0x457, mDifficulty, 1, i3, 8, 9, 0x58, false, false, true
        );
        CampaignProgress &progress = pProfile->AccessCampaignProgress(mDifficulty);
        Symbol era_tan_battle("era_tan_battle");
        if (s1 != era_tan_battle) {
            if (!TheGameMode->InMode("campaign_intro")) {
                progress.UnlockAllMoves(s1, s2);
            }
        }
        progress.BookmarkCurrentProgress();
    }
    CheckForEraSongUnlock();
}

Symbol CampaignPerformer::GetLastEra() const {
    for (int i = 0; i < TheCampaign->NumEras(); i++) {
        CampaignEra *pEra = TheCampaign->GetEra(i);
        MILO_ASSERT(pEra, 0x3B);
        // if pEra->unk50
        return pEra->GetName();
    }
    return gNullStr;
}

bool CampaignPerformer::IsDanceCrazeMove(Symbol s1, Symbol s2, HamMove *move) {
    if (move) {
        CampaignEra *pEra = TheCampaign->GetCampaignEra(s1);
        MILO_ASSERT(pEra, 0x409);
        bool b3 = false;
        for (int i = 0; i < pEra->GetNumSongs(); i++) {
            Symbol songName = pEra->GetSongName(i);
            if (s2 == songName) {
                b3 = true;
                break;
            }
        }
        if (b3) {
            return pEra->HasCrazeMove(s2, move->Name());
        }
    }
    return false;
}

bool CampaignPerformer::IsDanceCrazeMoveMastered(Symbol s1, Symbol s2, HamMove *move) {
    if (!move) {
        return false;
    } else {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x426);
        const CampaignProgress &progress = pProfile->GetCampaignProgress(mDifficulty);
        bool ret = progress.IsMoveMastered(s1, s2, move->Name());
        if (!ret) {
            CampaignEra *pEra = TheCampaign->GetCampaignEra(s1);
            MILO_ASSERT(pEra, 0x430);
            Symbol hamMoveName = pEra->GetHamMoveNameFromVariant(s2, move->Name());
            if (!hamMoveName.Null()) {
                ret = progress.IsMoveMastered(s1, s2, hamMoveName);
            }
        }
        return ret;
    }
}

void CampaignPerformer::UpdateEraSong(Difficulty d, Symbol s2, Symbol s3, int i4) {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x158);
    mLastEraStars = pProfile->GetCampaignProgress(d).GetEraStarsEarned(s2);
    for (int i = 0; i <= d; i++) {
        CampaignProgress &curProgress = pProfile->AccessCampaignProgress((Difficulty)i);
        curProgress.UpdateEraSong(s2, s3, i4);
        if (IsCampaignComplete() && i == kDifficultyExpert) {
            static Symbol stars_earned("stars_earned");
            const DataNode *pStarsNode = TheHamProvider->Property(stars_earned, false);
            MILO_ASSERT(pStarsNode, 0x167);
            if (pStarsNode->Int() >= 5) {
                // curProgress.unk28++;
            }
        }
    }
    TheAccomplishmentMgr->CheckForCampaignAccomplishmentsForProfile(pProfile);
    for (int i = 0; i <= d; i++) {
        CheckForOutfitAwards((Difficulty)i, s2);
        CheckForMasteryGoal((Difficulty)i, s2);
    }
}

void CampaignPerformer::AwardCrazeAccomplishments() {
    for (int i = 0; i < TheCampaign->NumEras() - 1; i++) {
        CampaignEra *pEra = TheCampaign->GetEra(i);
        Symbol acc = pEra->CompletionAccomplishment();
        if (acc != gNullStr) {
            int starsEarned =
                GetSongStarsEarned(pEra->GetName(), pEra->GetDanceCrazeSong());
            if (starsEarned >= 5) {
                HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
                MILO_ASSERT(pProfile, 0x218);
                TheAccomplishmentMgr->EarnAccomplishmentForProfile(pProfile, acc, true);
            }
        }
    }
}

void CampaignPerformer::AwardMasterQuestAccomplishments() {
    if (IsCampaignComplete()) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        MILO_ASSERT(pProfile, 0x22E);
        static Symbol acc_5_star_on_1_master_quest("acc_5_star_on_1_master_quest");
        static Symbol acc_5_star_on_5_master_quest("acc_5_star_on_5_master_quest");
        static Symbol acc_5_star_on_10_master_quest("acc_5_star_on_10_master_quest");
        const CampaignProgress &progress =
            pProfile->GetCampaignProgress(kDifficultyExpert);
        if (progress.Num5StarredMQSongs() >= 1) {
            TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                pProfile, acc_5_star_on_1_master_quest, false
            );
        }
        if (progress.Num5StarredMQSongs() >= 5) {
            TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                pProfile, acc_5_star_on_5_master_quest, false
            );
        }
        if (progress.Num5StarredMQSongs() >= 10) {
            TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                pProfile, acc_5_star_on_10_master_quest, false
            );
        }
    }
}

int CampaignPerformer::GetSongAttemptedCount() {
    CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
    MILO_ASSERT(pEra, 0x457);
    int numSongs = pEra->GetNumSongs();
    int numAttemptedSongs = 0;
    for (int i = 0; i < numSongs; i++) {
        if (HasSongBeenAttempted(pEra->GetSongName(i))) {
            numAttemptedSongs++;
        }
    }
    return numAttemptedSongs;
}

void CampaignPerformer::SetupCampaignCharacters(Symbol s1, Symbol s2) {
    Symbol s50 = GetCrewCharacter(s1, 0);
    Symbol s4c = GetCrewCharacter(s1, 1);
    if (s2 == s4c) {
        s4c = s50;
        s50 = s2;
    }
    Symbol s48 = MakeString("%s04", s50.Str());
    if (!GetOutfitEntry(s48, false)) {
        s48 = MakeString("%s01", s50.Str());
        if (!GetOutfitEntry(s48, false)) {
            s48 = MakeString("%s05", s50.Str());
        }
    }
    Symbol s44 = MakeString("%s04", s4c.Str());
    if (!GetOutfitEntry(s44, false)) {
        s44 = MakeString("%s01", s4c.Str());
        if (!GetOutfitEntry(s44, false)) {
            s44 = MakeString("%s05", s4c.Str());
        }
    }
    if (GetSongAttemptedCount() == 0) {
        if (IsCampaignIntroComplete()) {
            static Symbol era01("era01");
            static Symbol era02("era02");
            static Symbol era03("era03");
            if (mEra == era01 || mEra == era02 || mEra == era03) {
                Symbol s2 = s44;
                Symbol s1 = s4c;
                s4c = s50;
                s50 = s1;
                s44 = s48;
                s48 = s2;
            }
        }
    }
    HamPlayerData *pPlayer1Data = TheGameData->Player(0);
    MILO_ASSERT(pPlayer1Data, 0xAD);
    HamPlayerData *pPlayer2Data = TheGameData->Player(1);
    MILO_ASSERT(pPlayer2Data, 0xAF);
    pPlayer1Data->SetCharacter(s50);
    pPlayer1Data->SetOutfit(s48);
    pPlayer2Data->SetCharacter(s4c);
    pPlayer2Data->SetOutfit(s44);
}
