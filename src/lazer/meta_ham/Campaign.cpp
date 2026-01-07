#include "meta_ham/Campaign.h"
#include "hamobj/HamLabel.h"
#include "hamobj/HamMove.h"
#include "meta_ham/CampaignEra.h"
#include "macros.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

Campaign *TheCampaign;
DataArray *s_pReloadedCampaignData;

static const char *sCampaignStateDescs[] = { "kCampaignStateInactive",
                                             "kCampaignStateProfileSelect",
                                             "kCampaignStateDiffSelect",
                                             "kCampaignStateIntroMovie",
                                             "kCampaignStateIntroDance",
                                             "kCampaignStateIntroRetry",
                                             "kCampaignStateIntroAbort",
                                             "kCampaignStateEraIntroMovie",
                                             "kCampaignStateSongSelect",
                                             "kCampaignStateModeSelect",
                                             "kCampaignStatePerformSetup",
                                             "kCampaignStatePracticeSetup",
                                             "kCampaignStateHollaback",
                                             "kCampaignStatePerformIt",
                                             "kCampaignStateBreakItDown",
                                             "kCampaignStateBidEndgame",
                                             "kCampaignStateResults",
                                             "kCampaignStatePostResults",
                                             "kCampaignStateDciCutscene",
                                             "kCampaignStateTanBattle",
                                             "kCampaignStateTanBattleComplete",
                                             "kCampaignStatePostCreditsGlitterati",
                                             "kCampaignStateMasterQuestCrewSelect",
                                             "kCampaignStateMasterQuestSongSelect",
                                             "kCampaignStateExit" };

Campaign::Campaign(DataArray *d)
    : mCampaignState(kCampaignStateInactive), mWorkItActive(false),
      mOutroIntroSeen(false), m_pCurLoader(0), unkbc(gNullStr), unkd8(0) {
    MILO_ASSERT(!TheCampaign, 0x41);
    TheCampaign = this;
    SetName("campaign", ObjectDir::Main());
    ConfigureCampaignData(d);
}

Campaign::~Campaign() {
    TheCampaign = nullptr;
    if (s_pReloadedCampaignData) {
        s_pReloadedCampaignData->Release();
    }
    Cleanup();
    RELEASE(unkd8);
}

BEGIN_HANDLERS(Campaign)
    HANDLE_ACTION(set_campaign_state, SetCurState((CampaignState)_msg->Int(2)))
    HANDLE_EXPR(get_campaign_state, mCampaignState)
    HANDLE_EXPR(get_campaign_state_desc, (Symbol)sCampaignStateDescs[_msg->Int(2)])
    HANDLE_EXPR(get_intro_song, GetIntroSong(_msg->Int(2)))
    HANDLE_EXPR(get_intro_song_character, GetIntroSongCharacter(_msg->Int(2)))
    HANDLE_EXPR(get_intro_song_stars, GetIntroSongStarsRequired(_msg->Int(2)))
    HANDLE_EXPR(get_intro_venue, mIntroVenue)
    HANDLE_EXPR(get_intro_crew, mIntroCrew)
    HANDLE_EXPR(get_outro_song, GetOutroSong(_msg->Int(2)))
    HANDLE_EXPR(get_outro_song_character, GetOutroSongCharacter(_msg->Int(2)))
    HANDLE_EXPR(get_outro_song_stars_required, GetOutroSongStarsRequired(_msg->Int(2)))
    HANDLE_ACTION(
        reset_outro_song_stars_earned, ResetOutroStarsEarnedStartingAtIndex(_msg->Int(2))
    )
    HANDLE_EXPR(get_outro_song_gameplay_mode, GetOutroSongGameplayMode(_msg->Int(2)))
    HANDLE_EXPR(
        get_outro_song_fail_restart_index, GetOutroSongFailRestartIndex(_msg->Int(2))
    )
    HANDLE_EXPR(get_outro_song_rehearse_allowed, GetOutroSongRehearseAllowed(_msg->Int(2)))
    HANDLE_EXPR(get_outro_song_shortened, GetOutroSongShortened(_msg->Int(2)))
    HANDLE_EXPR(
        get_outro_song_freestyle_enabled, GetOutroSongFreestyleEnabled(_msg->Int(2))
    )
    HANDLE_ACTION(set_master_quest_crew, mMasterQuestCrew = _msg->Sym(2))
    HANDLE_EXPR(get_master_quest_crew, mMasterQuestCrew)
    HANDLE_ACTION(set_master_quest_song, mMasterQuestSong = _msg->Sym(2))
    HANDLE_EXPR(get_master_quest_song, mMasterQuestSong)
    HANDLE_EXPR(num_campaign_moves, (int)(mCampaignMoves.size())) // idk
    HANDLE_EXPR(num_campaign_song_moves, NumCampaignSongMoves(_msg->Sym(2))) // idk
    HANDLE_EXPR(
        get_campaign_ham_move,
        dynamic_cast<HamMove *>((GetHamMove(_msg->Sym(2), _msg->Int(3))))
    )
    HANDLE_EXPR(get_campaign_move_name, GetMoveName(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(get_outro_intro_seen, mOutroIntroSeen)
    HANDLE_ACTION(set_outro_intro_seen, mOutroIntroSeen = _msg->Int(2))
    HANDLE_ACTION(cheat_reload_data, CheatReloadData())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(Campaign)
    SYNC_PROP(work_it_active, mWorkItActive);
    SYNC_SUPERCLASS(Hmx::Object);
END_PROPSYNCS

void Campaign::FinishLoading(Loader *l) {
    DirLoader *dl = dynamic_cast<DirLoader *>(l);
    MILO_ASSERT(dl == m_pCurLoader, 0x315);
    MILO_ASSERT(m_pCurLoader->IsLoaded(), 0x316);
    unkd8 = m_pCurLoader->GetDir();
    FOREACH (it, mCampaignMoves) {
        CampaignMove *pActiveLoad = *it;
        MILO_ASSERT(pActiveLoad, 0x31D);
        HamMove *move = unkd8->Find<HamMove>(pActiveLoad->mMoveVariantName.Str(), false);
        if (move) {
            pActiveLoad->mMove = move;
            pActiveLoad->unk10 = 3;
        } else {
            MILO_NOTIFY(
                "Loading Campaign Move Data: move '%s' not found in %s!",
                pActiveLoad->mMoveName.Str(),
                pActiveLoad->mMoveVariantName.Str()
            );
            pActiveLoad->mMove = nullptr;
            pActiveLoad->unk10 = 2;
        }
    }
    RELEASE(m_pCurLoader);
}

void Campaign::FailedLoading(Loader *l) {
    DirLoader *dl = dynamic_cast<DirLoader *>(l);
    MILO_ASSERT(dl == m_pCurLoader, 0x337);
    RELEASE(m_pCurLoader);
    MILO_NOTIFY("Loading Campaign Move Data: move data not found");
    FOREACH (it, mCampaignMoves) {
        CampaignMove *move = *it;
        move->unk10 = 2;
        move->mMove = nullptr;
    }
}

void Campaign::SetCurState(CampaignState state) {
    char const *currState = sCampaignStateDescs[mCampaignState];
    char const *addState = sCampaignStateDescs[state];
    MILO_LOG(
        "[==============================================================================] %-32s -> %s\n",
        currState,
        addState
    );
    mCampaignState = state;
}

Symbol Campaign::GetIntroVenue() const { return mIntroVenue; }
Symbol Campaign::GetIntroCrew() const { return mIntroCrew; }

int Campaign::GetMaxStars() const {
    int stars = 0;
    FOREACH (it, m_vEras) {
        CampaignEra *pEra = *it;
        MILO_ASSERT(pEra, 0x236);
        stars += pEra->GetMaxStars();
    }
    return stars;
}

int Campaign::GetNumIntroSongs() const { return m_vIntroSongs.size(); }
int Campaign::GetNumOutroSongs() const { return m_vOutroSongs.size(); }

void Campaign::ResetOutroStarsEarnedStartingAtIndex(int iIndex) {
    for (int i = iIndex; i < m_vOutroSongs.size(); i++) {
        m_vOutroSongs[i]->mStarsEarned = 0;
    }
}

Symbol Campaign::GetCampaignWinInstructions(int iIndex) const {
    iIndex--;
    MILO_ASSERT_RANGE(iIndex, 0, m_vInstructions.size(), 0x248);
    return mWinInstructions[iIndex];
}

Symbol Campaign::GetIntroSong(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vIntroSongs.size(), 0x259);
    return m_vIntroSongs[i_iIndex]->mIntroSong;
}

Symbol Campaign::GetIntroSongCharacter(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vIntroSongs.size(), 0x25f);
    return m_vIntroSongs[i_iIndex]->mCharacter;
}

int Campaign::GetIntroSongStarsRequired(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vIntroSongs.size(), 0x265);
    return m_vIntroSongs[i_iIndex]->mStarsRequired;
}

Symbol Campaign::GetOutroSong(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x279);
    return m_vOutroSongs[i_iIndex]->mOutroSong;
}

Symbol Campaign::GetOutroSongCharacter(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x27f);
    return m_vOutroSongs[i_iIndex]->mCharacter;
}

int Campaign::GetOutroSongStarsRequired(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x285);
    return m_vOutroSongs[i_iIndex]->mStarsRequired;
}

Symbol Campaign::GetOutroSongGameplayMode(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x28b);
    return m_vOutroSongs[i_iIndex]->mGameplayMode;
}

int Campaign::GetOutroSongFailRestartIndex(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x291);
    return m_vOutroSongs[i_iIndex]->mFailRestartIndex;
}

bool Campaign::GetOutroSongRehearseAllowed(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x297);
    return m_vOutroSongs[i_iIndex]->mRehearseAllowed;
}

bool Campaign::GetOutroSongShortened(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x29d);
    return m_vOutroSongs[i_iIndex]->mSongShortened;
}

bool Campaign::GetOutroSongFreestyleEnabled(int i_iIndex) const {
    MILO_ASSERT(i_iIndex < m_vOutroSongs.size(), 0x2a3);
    return m_vOutroSongs[i_iIndex]->mFreestyleEnabled;
}

int Campaign::GetOutroStarsEarned(int iIndex) const {
    if (0 <= iIndex && iIndex < m_vOutroSongs.size())
        return m_vOutroSongs[iIndex]->mStarsEarned;
    return 0;
}

void Campaign::SetOutroStarsEarned(int i1, int i2) {
    if (i1 >= 0 && i1 < m_vOutroSongs.size() && m_vOutroSongs[i1]->mStarsEarned < i2) {
        m_vOutroSongs[i1]->mStarsEarned = i2;
    }
}

CampaignEra *Campaign::GetCampaignEra(Symbol era) const {
    auto it = mEraLookup.find(era);
    if (it != mEraLookup.end()) {
        return GetCampaignEra(it->second);
    }
    return nullptr;
}

CampaignEra *Campaign::GetCampaignEra(int i_iIndex) const {
    MILO_ASSERT_RANGE(i_iIndex, 0, m_vEras.size(), 0x106);
    return m_vEras[i_iIndex];
}

void Campaign::LoadHamMoves(Symbol s) {
    MILO_ASSERT(m_pCurLoader==NULL, 0x347);
    unkb8 = false;
    RELEASE(unkd8);
    FilePath movesPath(MakeString("modular_song_data/%s_moves.milo", s.Str()));
    m_pCurLoader =
        new DirLoader(movesPath, kLoadFront, this, nullptr, nullptr, false, nullptr);
}

void Campaign::GatherMoveData(Symbol sym) {
    CampaignEra *pEra = GetCampaignEra(sym);
    MILO_ASSERT(pEra, 0x2f0);
    int numSongsForEra = pEra->GetNumSongs();
    Symbol danceCrazeSong = pEra->GetDanceCrazeSong();
    for (int i = 0; i < numSongsForEra; i++) {
        CampaignEraSongEntry *pSongEntry = pEra->GetSongEntry(i);
        MILO_ASSERT(pSongEntry, 0x2f8);
        Symbol songEntryName = pSongEntry->GetSongName();
        if (songEntryName != danceCrazeSong) {
            int numSongCrazeMoves = pSongEntry->GetNumSongCrazeMoves();
            for (int j = 0; j < numSongCrazeMoves; j++) {
                Symbol crazeMoveName = pSongEntry->GetCrazeMoveName(j);
                Symbol crazeMoveVariantName = pSongEntry->GetCrazeMoveVariantName(j);
                CampaignMove *move =
                    new CampaignMove(songEntryName, crazeMoveName, crazeMoveVariantName);
                mCampaignMoves.push_back(move);
            }
        }
    }
}

void Campaign::Cleanup() {
    FOREACH (it, m_vEras) {
        RELEASE(*it);
    }
    FOREACH (it, m_vIntroSongs) {
        if (*it) {
            delete *it;
        }
        *it = nullptr;
    }
    FOREACH (it, m_vOutroSongs) {
        if (*it) {
            delete *it;
        }
        *it = nullptr;
    }
    m_vEras.clear();
    m_vIntroSongs.clear();
    m_vOutroSongs.clear();
    mEraLookup.clear();
    m_vInstructions.clear();
    mWinInstructions.clear();
}

int Campaign::NumCampaignSongMoves(Symbol s) {
    int moves = 0;
    FOREACH (it, mCampaignMoves) {
        CampaignMove *move = *it;
        if (move->unk10 == 3 && move->mSongName == s)
            moves++;
    }
    return moves;
}

void Campaign::ConfigureCampaignData(DataArray *i_pConfig) {
    static Symbol venue("venue");
    static Symbol crew("crew");
    static Symbol songs("songs");

    mIntroVenue = gNullStr;
    mIntroCrew = gNullStr;

    static Symbol campaign_intro("campaign_intro");
    DataArray *pIntroArray = i_pConfig->FindArray(campaign_intro);
    MILO_ASSERT(pIntroArray, 0x94);
    pIntroArray->FindData(venue, mIntroVenue, false);
    pIntroArray->FindData(crew, mIntroCrew, false);

    DataArray *pSongArray = pIntroArray->FindArray(songs);
    MILO_ASSERT(pSongArray, 0x99);
    MILO_ASSERT(pSongArray->Size() > 1, 0x9a);
    int numSongs = pSongArray->Size() - 1;
    for (int i = 0; i < numSongs; i++) {
        DataArray *curArr = pSongArray->Array(i + 1);
        Symbol s1 = curArr->Sym(0);
        Symbol s2 = curArr->Sym(1);
        int i3 = curArr->Int(2);
        CampaignIntroSong *song = new CampaignIntroSong(s1, s2, i3);
        m_vIntroSongs.push_back(song);
    }

    unk94 = gNullStr;
    unk98 = gNullStr;
    static Symbol campaign_outro("campaign_outro");
    DataArray *pOutroArray = i_pConfig->FindArray(campaign_outro);
    MILO_ASSERT(pOutroArray, 0xAB);
    pOutroArray->FindData(venue, unk94, false);
    pOutroArray->FindData(crew, unk98, false);
    pSongArray = pIntroArray->FindArray(songs);
    MILO_ASSERT(pSongArray, 0xB0);
    MILO_ASSERT(pSongArray->Size() > 1, 0xB1);
    numSongs = pSongArray->Size() - 1;
    for (int i = 0; i < numSongs; i++) {
        DataArray *curArr = pSongArray->Array(i + 1);
        Symbol s0 = curArr->Sym(0);
        Symbol s1 = curArr->Sym(1);
        int i2 = curArr->Int(2);
        Symbol s3 = curArr->Sym(3);
        int i4 = curArr->Int(4);
        bool i5 = curArr->Int(5);
        bool i6 = curArr->Int(6);
        bool i7 = curArr->Int(7);
        CampaignOutroSong *song = new CampaignOutroSong(s0, s1, i2, s3, i4, i5, i6, i7);
        m_vOutroSongs.push_back(song);
    }

    static Symbol eras("eras");
    DataArray *pEraArray = i_pConfig->FindArray(eras);
    MILO_ASSERT(pEraArray, 0xc6);

    int iNumEras = pEraArray->Size();
    if (pEraArray->Size() > 10) {
        MILO_NOTIFY("Too many campaign eras! ");
        iNumEras = 10;
    }

    static Symbol era_move_names_lookup("era_move_names_lookup");
    DataArray *pLookupEraArray = i_pConfig->FindArray(era_move_names_lookup);
    MILO_ASSERT(pLookupEraArray, 0xd1);
    MILO_ASSERT(iNumEras == pLookupEraArray->Size(), 0xd2);

    for (int i = 1; i < iNumEras; i++) {
        CampaignEra *pCampaignEra =
            new CampaignEra(pEraArray->Array(i), pLookupEraArray->Array(i));
        MILO_ASSERT(pCampaignEra, 0xd8);
        Symbol name = pCampaignEra->GetName();
        if (GetCampaignEra(name)) {
            MILO_NOTIFY("%s campaign era already exists, skipping", name.Str());
            delete pCampaignEra;
        } else {
            if (pCampaignEra->IsTanBattleEra()) {
                MILO_ASSERT(m_vEras.size(), 0xE4);
                GetCampaignEra(m_vEras.size() - 1); // ->unk50 = true;
            }
            mEraLookup[name] = m_vEras.size();
            m_vEras.push_back(pCampaignEra);
        }
    }

    static Symbol instructions("instructions");
    DataArray *pInstructionArray = i_pConfig->FindArray(instructions);
    MILO_ASSERT(pInstructionArray, 0xf0);
    for (int i = 1; i < pInstructionArray->Size(); i++) {
        Symbol s = pInstructionArray->Sym(i);
        m_vInstructions.push_back(s);
    }

    static Symbol win_instructions("win_instructions");
    DataArray *pWinInstructionArray = i_pConfig->FindArray(win_instructions);
    MILO_ASSERT(pWinInstructionArray, 0xfb);
    for (int i = 1; i < pWinInstructionArray->Size(); i++) {
        Symbol s = pWinInstructionArray->Sym(i);
        mWinInstructions.push_back(s);
    }
}

void Campaign::LoadCampaignDanceMoves(Symbol s) {
    if (s != unkbc) {
        mCampaignMoves.clear();
        unkbc = s;
        GatherMoveData(s);
        unkb8 = false;
        m_pCurLoader = nullptr;
        LoadHamMoves(s);
    }
}

void Campaign::CheatReloadData() {
    if (s_pReloadedCampaignData) {
        s_pReloadedCampaignData->Release();
    }
    s_pReloadedCampaignData = DataReadFile("config/campaign.dta", true);
    Cleanup();
    ConfigureCampaignData(s_pReloadedCampaignData);
}

HamMove *Campaign::GetHamMove(Symbol s1, int i2) {
    int i = -1;
    FOREACH (it, mCampaignMoves) {
        CampaignMove *curMove = *it;
        if (curMove->unk10 == 3 && curMove->mSongName == s1) {
            i++;
        }
        if (i == i2) {
            return curMove->mMove;
        }
    }
    return nullptr;
}

Symbol Campaign::GetMoveName(Symbol s1, int i2) {
    int i = -1;
    FOREACH (it, mCampaignMoves) {
        CampaignMove *curMove = *it;
        if (curMove->unk10 == 3 && curMove->mSongName == s1) {
            i++;
        }
        if (i == i2) {
            return curMove->mMoveName;
        }
    }
    return 0;
}

bool Campaign::UpdateEraSongUnlockInstructions(
    Symbol song, HamLabel *i_pInstructionsLabel
) {
    MILO_ASSERT(i_pInstructionsLabel, 0x131);
    CampaignPerformer *pPerformer =
        dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
    if (!pPerformer) {
        MILO_NOTIFY("Campaign::UpdateEraSongUnlockInstructions pPerformer is NULL");
        return false;
    }
    Symbol era = pPerformer->Era();
    CampaignEra *pEra = GetCampaignEra(era);
    if (!pEra) {
        MILO_ASSERT(pEra, 0x13D);
        MILO_NOTIFY("Campaign::UpdateEraSongUnlockInstructions pEra is NULL");
        return false;
    }
    int requiredStars = pEra->GetSongRequiredStars(song);
    if (requiredStars == 0) {
        static Symbol era01("era01");
        static Symbol era02("era02");
        static Symbol era03("era03");
        static Symbol era04("era04");
        if (era == era01) {
            static Symbol campaign_song_hint_70s("campaign_song_hint_70s");
            i_pInstructionsLabel->SetTextToken(campaign_song_hint_70s);
        } else if (era == era02) {
            static Symbol campaign_song_hint_80s("campaign_song_hint_80s");
            i_pInstructionsLabel->SetTextToken(campaign_song_hint_80s);
        } else if (era == era03) {
            static Symbol campaign_song_hint_90s("campaign_song_hint_90s");
            i_pInstructionsLabel->SetTextToken(campaign_song_hint_90s);
        } else if (era == era04) {
            static Symbol campaign_song_hint_00s("campaign_song_hint_00s");
            i_pInstructionsLabel->SetTextToken(campaign_song_hint_00s);
        } else {
            static Symbol campaign_song_hint_10s("campaign_song_hint_10s");
            i_pInstructionsLabel->SetTextToken(campaign_song_hint_10s);
        }
        return false;
    }
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x173);
    const CampaignProgress &campaignProgress =
        pProfile->GetCampaignProgress(pPerformer->GetDifficulty());
    int starsEarned = campaignProgress.GetEraStarsEarned(era);
    int movesRequired = pEra->MovesRequiredForMastery();
    int i8 = Max(requiredStars - starsEarned, 0);
    int eraMovesMastered = campaignProgress.GetEraMovesMastered(era);
    if (eraMovesMastered >= movesRequired) {
        eraMovesMastered = movesRequired;
    }
    int i9 = Max(movesRequired - eraMovesMastered, 0);
    if (i8 > 1) {
        if (i9 > 1) {
            static Symbol campaign_song_hint_both("campaign_song_hint_both");
            i_pInstructionsLabel->SetTokenFmt(campaign_song_hint_both, i8, i9);
            return true;
        } else if (i9 == 1) {
            static Symbol campaign_song_hint_both_singular_move(
                "campaign_song_hint_both_singular_move"
            );
            i_pInstructionsLabel->SetTokenFmt(campaign_song_hint_both_singular_move, i8);
            return true;
        } else if (i9 == 0) {
            static Symbol campaign_song_hint("campaign_song_hint");
            i_pInstructionsLabel->SetTokenFmt(campaign_song_hint, i8);
            return true;
        }
    }
    if (i8 == 1) {
        if (i9 > 1) {
            static Symbol campaign_song_hint_both_singular_star(
                "campaign_song_hint_both_singular_star"
            );
            i_pInstructionsLabel->SetTokenFmt(campaign_song_hint_both_singular_star, i9);
            return true;
        }
        if (i9 == 1) {
            static Symbol campaign_song_hint_both_singular_both(
                "campaign_song_hint_both_singular_both"
            );
            i_pInstructionsLabel->SetTextToken(campaign_song_hint_both_singular_both);
            return true;
        } else if (i9 == 0) {
            static Symbol campaign_song_hint_singular("campaign_song_hint_singular");
            i_pInstructionsLabel->SetTextToken(campaign_song_hint_singular);
            return true;
        }
    } else if (i8 == 0) {
        if (i9 > 1) {
            static Symbol campaign_song_hint_moves("campaign_song_hint_moves");
            i_pInstructionsLabel->SetTokenFmt(campaign_song_hint_moves, i9);
            return true;
        }
        if (i9 == 1) {
            static Symbol campaign_song_hint_moves_singular(
                "campaign_song_hint_moves_singular"
            );
            i_pInstructionsLabel->SetTextToken(campaign_song_hint_moves_singular);
            return true;
        }
    }
    static Symbol campaign_song_hint_singular("campaign_song_hint_singular");
    i_pInstructionsLabel->SetTextToken(campaign_song_hint_singular);
    return true;
}
