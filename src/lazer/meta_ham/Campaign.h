#pragma once
#include "hamobj/HamLabel.h"
#include "hamobj/HamMove.h"
#include "meta_ham/CampaignEra.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"
#include <vector>

enum MoveLoadStatus {
    kMoveLoadStatusPending = 0x0000,
    kMoveLoadStatusActive = 0x0001,
    kMoveLoadStatusFailed = 0x0002,
    kMoveLoadStatusComplete = 0x0003
};

enum CampaignState {
    kCampaignStateInactive = 0,
    kCampaignStateProfileSelect = 1,
    kCampaignStateDiffSelect = 2,
    kCampaignStateIntroMovie = 3,
    kCampaignStateIntroDance = 4,
    kCampaignStateIntroRetry = 5,
    kCampaignStateIntroAbort = 6,
    kCampaignStateEraIntroMovie = 7,
    kCampaignStateSongSelect = 8,
    kCampaignStateModeSelect = 9,
    kCampaignStatePerformSetup = 10,
    kCampaignStatePracticeSetup = 11,
    kCampaignStateHollaback = 12,
    kCampaignStatePerformIt = 13,
    kCampaignStateBreakItDown = 14,
    kCampaignStateBidEndgame = 15,
    kCampaignStateResults = 16,
    kCampaignStatePostResults = 17,
    kCampaignStateDciCutscene = 18,
    kCampaignStateTanBattle = 19,
    kCampaignStateTanBattleComplete = 20,
    kCampaignStatePostCreditsGlitterati = 21,
    kCampaignStateMasterQuestCrewSelect = 22,
    kCampaignStateMasterQuestSongSelect = 23,
    kCampaignStateExit = 24,
    kNumCampaignStates = 25
};

class CampaignIntroSong {
public:
    CampaignIntroSong(Symbol song, Symbol charSym, int stars)
        : m_symSongName(song), m_symDefaultCharacter(charSym),
          m_iMinStarsToMaster(stars) {}

    Symbol m_symSongName; // 0x0
    Symbol m_symDefaultCharacter; // 0x4
    int m_iMinStarsToMaster; // 0x8
};

class CampaignOutroSong {
public:
    CampaignOutroSong(
        Symbol song,
        Symbol charSym,
        int stars,
        Symbol mode,
        int index,
        bool bid,
        bool shortened,
        bool freestyle
    )
        : m_symSongName(song), m_symDefaultCharacter(charSym), m_iStarsEarned(0),
          m_iMinStarsToMaster(stars), m_symGameplayMode(mode), m_iFailRestartIndex(index),
          m_bRehearseAllowed(bid), m_bShortenSong(shortened),
          m_bFreestyleEnabled(freestyle) {}

    Symbol m_symSongName; // 0x0
    Symbol m_symDefaultCharacter; // 0x4
    int m_iStarsEarned; // 0x8
    int m_iMinStarsToMaster; // 0xc
    Symbol m_symGameplayMode; // 0x10
    int m_iFailRestartIndex; // 0x14
    bool m_bRehearseAllowed; // 0x18
    bool m_bShortenSong; // 0x19
    bool m_bFreestyleEnabled; // 0x1a
};

class CampaignMove {
public:
    CampaignMove(Symbol s1, Symbol s2, Symbol s3)
        : m_symSongName(s1), m_symMoveName(s2), m_symMoveVariantName(s3), m_pHamMove(0),
          m_LoadStatus(kMoveLoadStatusPending) {}
    Symbol m_symSongName; // 0x0
    Symbol m_symMoveName; // 0x4
    Symbol m_symMoveVariantName; // 0x8
    HamMove *m_pHamMove; // 0xc
    MoveLoadStatus m_LoadStatus; // 0x10
};

class Campaign : public Hmx::Object, public Loader::Callback {
public:
    Campaign(DataArray *);
    // Hmx::Object
    virtual ~Campaign();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // Loader::Callback
    virtual void FinishLoading(Loader *);
    virtual void FailedLoading(Loader *);

    Symbol GetIntroVenue() const;
    Symbol GetIntroCrew() const;
    void SetCurState(CampaignState);
    int GetMaxStars() const;
    int GetNumIntroSongs() const;
    int GetNumOutroSongs() const;
    void ResetOutroStarsEarnedStartingAtIndex(int);
    Symbol GetCampaignWinInstructions(int) const;
    Symbol GetIntroSong(int) const;
    Symbol GetIntroSongCharacter(int) const;
    int GetIntroSongStarsRequired(int) const;
    Symbol GetOutroSong(int) const;
    Symbol GetOutroSongCharacter(int) const;
    int GetOutroSongStarsRequired(int) const;
    Symbol GetOutroSongGameplayMode(int) const;
    int GetOutroSongFailRestartIndex(int) const;
    bool GetOutroSongRehearseAllowed(int) const;
    bool GetOutroSongShortened(int) const;
    bool GetOutroSongFreestyleEnabled(int) const;
    int GetOutroStarsEarned(int) const;
    void SetOutroStarsEarned(int, int);
    CampaignEra *GetCampaignEra(Symbol era) const;
    bool UpdateEraSongUnlockInstructions(Symbol, HamLabel *);
    void LoadCampaignDanceMoves(Symbol);
    void CheatReloadData();
    int NumCampaignSongMoves(Symbol s);

    CampaignEra *GetFirstEra() { return m_vEras.front(); }
    CampaignEra *GetLastEra() { return m_vEras.back(); }
    CampaignEra *GetEra(int i) { return m_vEras[i]; }
    int NumEras() const { return m_vEras.size(); }
    const std::vector<CampaignEra *> &GetCampaignEraVector() const { return m_vEras; }
    Symbol GetMasterQuestCrew() const { return m_symMasterQuestCrew; }
    bool InDCICutscene() const { return m_curState == kCampaignStateDciCutscene; }
    void SetMasterQuestCrewComplete(Symbol key, bool b) {
        mMasterQuestCrewComplete[key] = b;
    }

protected:
    CampaignEra *GetCampaignEra(int index) const;
    void LoadHamMoves(Symbol);
    HamMove *GetHamMove(Symbol, int);
    Symbol GetMoveName(Symbol, int);
    void GatherMoveData(Symbol);
    void Cleanup();
    void ConfigureCampaignData(DataArray *);

private:
    CampaignState m_curState; // 0x30
    bool m_WorkItActive; // 0x34
    // key = era symbol, value = index into m_vEras
    std::map<Symbol, int> m_mapEraLookup; // 0x38
    std::vector<CampaignEra *> m_vEras; // 0x50
    std::vector<Symbol> m_vInstructions; // 0x5c
    std::vector<Symbol> m_vWinInstructions; // 0x68
    std::vector<CampaignIntroSong *> m_vIntroSongs; // 0x74
    Symbol m_IntroVenue; // 0x80
    Symbol m_IntroCrew; // 0x84
    std::vector<CampaignOutroSong *> m_vOutroSongs; // 0x88
    Symbol m_OutroVenue; // 0x94
    Symbol m_OutroCrew; // 0x98
    bool m_OutroIntroSeen; // 0x9c
    Symbol m_symMasterQuestCrew; // 0xa0
    Symbol m_symMasterQuestSong; // 0xa4
    std::vector<CampaignMove *> m_Moves; // 0xa8
    DirLoader *m_pCurLoader; // 0xb4
    bool m_bAllMovesKLoaded; // 0xb8
    Symbol m_currentlyLoadedEra; // 0xbc
    // key = crew, value = ???
    std::map<Symbol, bool> mMasterQuestCrewComplete; // 0xc0
    ObjectDir *mCurLoaderDir; // 0xd8 not in pdb
};

extern Campaign *TheCampaign;
extern DataArray *s_pReloadedCampaignData;
