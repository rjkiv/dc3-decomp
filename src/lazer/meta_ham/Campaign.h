#pragma once

#include "hamobj/HamLabel.h"
#include "hamobj/HamMove.h"
#include "lazer/meta_ham/CampaignEra.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/Loader.h"
#include "utl/NetCacheMgr.h"
#include "utl/Symbol.h"

enum CampaignState {
    state1 = 0
};

class CampaignIntroSong {
public:
    Symbol mIntroSong; // 0x0
    Symbol mCharacter; // 0x4
    int mStarsRequired; // 0x8
};

class CampaignOutroSong {
public:
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
    Symbol unk0;
    int unk4;
    Symbol unk8;
    int unkc;
    int unk10;
};

class Campaign : public Hmx::Object, public Loader::Callback {
public:
    // Hmx::Object
    virtual ~Campaign();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    // Loader::Callback
    virtual void FinishLoading(Loader *);
    virtual void FailedLoading(Loader *);

    Campaign(DataArray *);
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
    CampaignEra *GetCampaignEra(Symbol) const;
    bool UpdateEraSongUnlockInstructions(Symbol, HamLabel *);
    void LoadCampaignDanceMoves(Symbol);
    void CheatReloadData();
    int NumCampaignSongMoves(Symbol s);

    CampaignEra *GetFrontEra() { return m_vEras.front(); }
    CampaignEra *GetBackEra() { return m_vEras.back(); }
    CampaignEra *GetEraAtIndex(int i) { return m_vEras[i]; }
    bool IsErasEmpty() { return m_vEras.empty(); }
    Symbol GetMQCrew() { return mMasterQuestCrew; }

    static char const *sCampaignStateDesc[];

protected:
    CampaignState unk30;
    bool unk34;
    std::map<Symbol, int> unk38;
    std::vector<CampaignEra *> m_vEras; // 0x50
    std::vector<Symbol> m_vInstructions; // 0x5c
    std::vector<Symbol> unk68;
    std::vector<CampaignIntroSong *> m_vIntroSongs; // 0x74
    Symbol mIntroVenue; // 0x80
    Symbol mIntroCrew; // 0x84
    std::vector<CampaignOutroSong *> m_vOutroSongs; // 0x88
    Symbol unk94;
    Symbol unk98;
    bool mIntroOutroSeen; // 0x9c
    Symbol mMasterQuestCrew; // 0xa0
    Symbol mMasterQuestSong; // 0xa4
    std::vector<CampaignMove *> unka8;
    DirLoader *m_pCurLoader; // 0xb4
    bool unkb8;
    Symbol unkbc;
    std::map<Symbol, bool> unkc0;
    ObjectDir *unkd8;

    CampaignEra *GetCampaignEra(int) const;
    void LoadHamMoves(Symbol);
    HamMove *GetHamMove(Symbol, int);
    Symbol GetMoveName(Symbol, int);
    void GatherMoveData(Symbol);
    void Cleanup();
    void ConfigureCampaignData(DataArray *);
};

extern Campaign *TheCampaign;
extern DataArray *s_pReloadedCampaignData;
