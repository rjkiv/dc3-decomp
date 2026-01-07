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
        : mIntroSong(song), mCharacter(charSym), mStarsRequired(stars) {}

    Symbol mIntroSong; // 0x0
    Symbol mCharacter; // 0x4
    int mStarsRequired; // 0x8
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
        : mOutroSong(song), mCharacter(charSym), mStarsEarned(0), mStarsRequired(stars),
          mGameplayMode(mode), mFailRestartIndex(index), mRehearseAllowed(bid),
          mSongShortened(shortened), mFreestyleEnabled(freestyle) {}

    Symbol mOutroSong; // 0x0
    Symbol mCharacter; // 0x4
    int mStarsEarned; // 0x8
    int mStarsRequired; // 0xc
    Symbol mGameplayMode; // 0x10
    int mFailRestartIndex; // 0x14
    bool mRehearseAllowed; // 0x18
    bool mSongShortened; // 0x19
    bool mFreestyleEnabled; // 0x1a
};

class CampaignMove {
public:
    CampaignMove(Symbol s1, Symbol s2, Symbol s3)
        : mSongName(s1), mMoveName(s2), mMoveVariantName(s3), mMove(0), unk10(0) {}
    Symbol mSongName; // 0x0
    Symbol mMoveName; // 0x4
    Symbol mMoveVariantName; // 0x8
    HamMove *mMove; // 0xc
    int unk10; // 0x10 - state?
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
    const std::vector<CampaignEra *> &Eras() const { return m_vEras; }
    Symbol GetMQCrew() { return mMasterQuestCrew; }
    bool InDCICutscene() const { return mCampaignState == kCampaignStateDciCutscene; }

protected:
    CampaignEra *GetCampaignEra(int index) const;
    void LoadHamMoves(Symbol);
    HamMove *GetHamMove(Symbol, int);
    Symbol GetMoveName(Symbol, int);
    void GatherMoveData(Symbol);
    void Cleanup();
    void ConfigureCampaignData(DataArray *);

    CampaignState mCampaignState; // 0x30
    bool mWorkItActive; // 0x34
    // key = era symbol, value = index into m_vEras
    std::map<Symbol, int> mEraLookup; // 0x38
    std::vector<CampaignEra *> m_vEras; // 0x50
    std::vector<Symbol> m_vInstructions; // 0x5c
    std::vector<Symbol> mWinInstructions; // 0x68
    std::vector<CampaignIntroSong *> m_vIntroSongs; // 0x74
    Symbol mIntroVenue; // 0x80
    Symbol mIntroCrew; // 0x84
    std::vector<CampaignOutroSong *> m_vOutroSongs; // 0x88
    Symbol unk94;
    Symbol unk98;
    bool mOutroIntroSeen; // 0x9c
    Symbol mMasterQuestCrew; // 0xa0
    Symbol mMasterQuestSong; // 0xa4
    std::vector<CampaignMove *> mCampaignMoves; // 0xa8
    DirLoader *m_pCurLoader; // 0xb4
    bool unkb8;
    Symbol unkbc;
    // key = crew, value = ???
    std::map<Symbol, bool> unkc0;
    ObjectDir *unkd8;
};

extern Campaign *TheCampaign;
extern DataArray *s_pReloadedCampaignData;
