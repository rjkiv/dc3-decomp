#include "meta_ham/CampaignPerformer.h"
#include "SaveLoadManager.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPlayerData.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/Campaign.h"
#include "meta_ham/CampaignEra.h"
#include "meta_ham/CampaignProgress.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Symbol.h"

CampaignPerformer::CampaignPerformer(HamSongMgr const &hsm)
    : MetaPerformer(hsm, "campaign_performer"), mEra(gNullStr),
      mDifficulty(kDifficultyEasy), mJustFinishedEra(false), mJustUnlockedEraSong(false),
      mWasLastMoveMastered(false), mLastEraStars(-1), mLastEraMoves(-1),
      mStarsEarnedSoFar(0) {}

BEGIN_PROPSYNCS(CampaignPerformer)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

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

void CampaignPerformer::SelectSong(Symbol song, int i) {
    TheGameData->SetSong(song);
    Symbol crew, introCrew;
    mJustUnlockedEraSong = false;
    mStarsEarnedSoFar = 0;
    if (TheGameMode->InMode("campaign_perform", true)) {
        BookmarkCurrentProgress();
        CampaignEra *pEra = TheCampaign->GetCampaignEra(mEra);
        MILO_ASSERT(pEra, 0x58);
        Symbol crew = pEra->Crew();
        CampaignEraSongEntry *pSongEntry = pEra->GetSongEntry(song);
        MILO_ASSERT(pSongEntry, 0x5c);
        introCrew = pSongEntry->unk8;
        crew = pEra->Crew();
    } else if (TheGameMode->InMode("campaign_intro", true)) {
        static Symbol era_tan_battle("era_tan_battle");
        if (mEra == era_tan_battle)
            return;
        crew = TheCampaign->GetIntroCrew();
        HamPlayerData *pPlayer1Data = TheGameData->Player(0);
        MILO_ASSERT(pPlayer1Data, 0x68);
        HamPlayerData *pPlayer2Data = TheGameData->Player(1);
        MILO_ASSERT(pPlayer2Data, 0x6a);
        introCrew = TheCampaign->GetIntroSongCharacter(i);
    }
    SetupCampaignCharacters(crew, introCrew);
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
    Symbol s = pEra->OutfitAward();
    if (s != gNullStr
        && pCampaignProgress.GetEraStarsEarned(era) < pEra->StarsRequiredForOutfits())
        return false;
    return true;
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
    return pEra->GetSongName(pEra->GetNumSongs() != 0); // look into this line
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
        CampaignProgress &pCampaignProgress =
            pProfile->AccessCampaignProgress((Difficulty)i);
        pCampaignProgress.BookmarkCurrentProgress();
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
    CampaignEra *pEra = TheCampaign->GetFrontEra();
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
