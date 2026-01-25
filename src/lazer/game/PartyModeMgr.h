#pragma once
#include "hamobj/Difficulty.h"
#include "math/DoubleExponentialSmoother.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/Playlist.h"
#include "net_ham/PartyModeJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/DateTime.h"
#include "utl/MemMgr.h"
#include "utl/PseudoRandomPicker.h"

DECLARE_MESSAGE(SmartGlassMsg, "smart_glass_msg")
END_MESSAGE

class PartyModeARObject {
public:
    PartyModeARObject(DataArray *a) : unk8(a) {}
    const char *GetTexPath();

    MEM_OVERLOAD(PartyModeARObject, 0x4C);

    int unk0;
    int unk4;
    DataArray *unk8;
};

class PartyModePlayer {
public:
    PartyModePlayer(PartyModeARObject *);
    ~PartyModePlayer();

    void PushTitle(Symbol);
    int Index() const { return unk1c; }
    void IncScore(int score) { unk14 += score; }
    void StoreFramePos(float x, float y) {
        unk20 = x;
        unk24 = y;
    }
    void StoreFrameScale(float scale) { unk28 = scale; }
    const char *GetTexPath() { return unk0->GetTexPath(); }
    int GetPhotoIndex() const { return unk2c; }
    void SetIndex(int idx) { unk1c = idx; }
    void SetSym(Symbol s) { unk4 = s; } // rename once context known
    void SetPhotoIndex(int idx) { unk2c = idx; }

private:
    PartyModeARObject *unk0;
    Symbol unk4;
    std::list<Symbol> unk8;
    DataArray *unk10;
    int unk14;
    int unk18;
    int unk1c;
    float unk20;
    float unk24;
    float unk28;
    int unk2c;
};

class PartyModeMgr : public Hmx::Object, public ContentMgr::Callback {
public:
    struct ConfigHistory {
        int mTimeStamp; // 0x0 - system ms
        Symbol mSong; // 0x4 - song
        Symbol mMode; // 0x8 - mode
        bool mForceCrewOutfit; // 0xc
    };
    struct SubMode {
        SubMode() : mPlayers(0) {}
        ~SubMode() {
            if (mPlayers) {
                mPlayers->Release();
            }
        }

        MEM_OVERLOAD(SubMode, 0xE3);

        Symbol mModeName; // 0x0 - mode name
        Symbol mSubModeName; // 0x4 - sub mode name
        Symbol mSongName; // 0x8 - song shortname
        int mSongID; // 0xc
        int mPlayerFlags; // 0x10
        int mNumPlayers; // 0x14
        int unk18;
        std::vector<int> unk1c;
        DataArray *mPlayers; // 0x28
    };
    PartyModeMgr();
    // Hmx::Object
    virtual ~PartyModeMgr();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // ContentMgr::Callback
    virtual void ContentMounted(const char *contentName, const char *);

    static void Init();

    void ResetPlayers();
    void AddPlayerToTeam(int);
    void FinalizeTeam(int);
    void ClearTeam(int);
    void FinalizeParty();
    void FinalizePlaytestParty();
    void StorePlayerFramePos(int, float, float);
    void StorePlayerFrameScale(int, float);
    const char *GetPlayerARTexPath(int);
    void ResetParty();
    void CrewShowdownRematch();
    int GetPlayerPhotoIndex(int);
    int GetLeftPlayerIndex() const;
    int GetRightPlayerIndex() const;
    void IncLeftPlayerScore(int);
    void IncRightPlayerScore(int);
    void PushLeftPlayerTitle(Symbol);
    void PushRightPlayerTitle(Symbol);
    void SetLeftTeamScore(float);
    void SetRightTeamScore(float);
    void IncLeftTeamScore(float);
    void IncRightTeamScore(float);
    void StartNewRound();
    float GetPointsForWin();
    float GetPointsForLoss();
    void ToggleIncludedModeOn(Symbol, bool);
    bool IsModeIncluded(Symbol);
    DataArray *GetLeftCrewColor1AsArray();
    DataArray *GetLeftCrewColor2AsArray();
    DataArray *GetRightCrewColor1AsArray();
    DataArray *GetRightCrewColor2AsArray();
    int GetCrewColor(int, int);
    bool LeftTeamMaxWins() const;
    bool RightTeamMaxWins() const;
    void SetLeftTeamStarBonus();
    void SetRightTeamStarBonus();
    void SetPlaylist(Playlist *);

    Difficulty GetDifficulty() const { return mDifficulty; }
    void SetDifficulty(Difficulty d) { mDifficulty = d; }
    bool UseFullLengthSongs() const { return mUseFullLengthSongs; }
    void SetUseFullLengthSongs(bool b) { mUseFullLengthSongs = b; }
    Playlist *GetPlaylist() const { return mPlaylist; }

private:
    void InitCharacters();
    Symbol GetCurrEventName();
    Symbol GetCurrEventMicrogameName();
    Symbol GetCurrEventSongName();
    Symbol GetCurrEventSongShortName();
    int GetCurrEventPlayerFlags();
    int GetCurrEventNumPlayers();
    Symbol GetCurrEventSongArtistName();
    DataArray *GetCurrEventPlayers();
    void SetCurrEvent();
    void UpdateRoundsPlayed();
    void SetRandomCharacters();
    void SetupCharacterData();
    bool IsTeamSignedIn(int);
    void SmoothFrameMotion(int, float, float);
    void ForceFrameSmootherPos(int, float, float);
    void UpdateScores();
    void UseSelectedPlaylist(bool);
    void ShufflePlaylist(bool);
    void ToggleIncludedMode(Symbol);
    void SetModes();
    void SetupInfinitePartyMode();
    const char *GetPlaylistString();
    Symbol GetLeftCrewCharOutfit(int, int);
    Symbol GetRightCrewCharOutfit(int, int);
    void SendPartyOptionsToRC();
    void GetPartyOptionsFromRC();
    void GetPartySongQueueFromRC();
    Symbol GetNextSongName();
    void ChangeToAnotherGameMode();
    void EndPartyStats();
    void OnSmartGlassListen(int);
    void PruneHistory();
    HamProfile *GetValidProfile();
    void BroadcastSyncMsg(Symbol);
    void ReadPartyOptions();
    void DeleteSongFromRCPartySongQueue(int);
    void AddNextSongToRCPartySongQueue();
    Symbol GetNextMode();
    void DetermineSubMode(Symbol *, Symbol *);
    void DetermineSubModeSong(Symbol *, int *);
    PartyModePlayer *CreatePartyModePlayer();
    void ResetMicrogames();
    int PickNextPlayer();
    void SetSongsFromPlaylist();
    void ResetSongs();
    void ResetModes(bool);
    void SetSongAndDefaults(Symbol, Symbol, bool);
    SubMode *CreateEventA();
    void DetermineSubModePlayers(Symbol, int *, int *, std::vector<int> *);

    Symbol GetCurrEventDisplayName() { return GetCurrEventName(); }
    bool IsUsingPlaylist() const { return mPlaylist; }

    DataNode OnGetSmoothedFramePos(const DataArray *);
    DataNode OnSetSongAndDefaults(DataArray *);
    DataNode OnStableSong();
    DataNode OnStableMode();
    DataNode OnMsg(const RCJobCompleteMsg &);
    DataNode OnMsg(const SmartGlassMsg &);

    DataArray *mPartyModeCfg; // 0x30
    bool mUsePlaytestData; // 0x34
    DataArray *mPartyModePlaytestData; // 0x38
    DataArray *mPartyModePlaytestEvents; // 0x3c
    bool unk40;
    SubMode *mCurrEvent; // 0x44
    int mRoundsPlayed; // 0x48
    int mRoundsTotal; // 0x4c
    int mRoundsUntilShowdown; // 0x50
    bool mIsShowdown; // 0x54
    DataArray *mARObjects; // 0x58
    std::vector<PartyModePlayer *> mPlayers; // 0x5c
    std::vector<PartyModePlayer *> mTeam1Players; // 0x68
    std::vector<PartyModePlayer *> mTeam2Players; // 0x74
    float mLeftTeamPrevScore; // 0x80
    float mLeftTeamScore; // 0x84
    float mRightTeamPrevScore; // 0x88
    float mRightTeamScore; // 0x8c
    float mLeftTeamStarBonus; // 0x90
    float mRightTeamStarBonus; // 0x94
    DataArray *mEventScoring; // 0x98
    int mWinningSide; // 0x9c
    int mJustWonSide; // 0xa0
    std::vector<int> unka4;
    std::vector<int> unkb0;
    PseudoRandomPicker<int> unkbc;
    PseudoRandomPicker<int> unkd0; // 0xd0 - for team 1
    PseudoRandomPicker<int> unke4; // 0xe4 - for team 2
    PseudoRandomPicker<Symbol> unkf8;
    PseudoRandomPicker<Symbol> mModePicker; // 0x10c
    PseudoRandomPicker<Symbol> mSubModePicker; // 0x120
    PseudoRandomPicker<Symbol> mGoodTitlePicker; // 0x134
    PseudoRandomPicker<Symbol> mBadTitlePicker; // 0x148
    // indexed by dj intensity rank
    PseudoRandomPicker<Symbol> mSubModeSongPickers[4]; // 0x15c
    std::vector<Symbol> mCharacters; // 0x1ac
    Symbol mLeftTeamCrew; // 0x1b8
    Symbol mRightTeamCrew; // 0x1bc
    PartyModePlayer *mLeftPlayer; // 0x1c0
    PartyModePlayer *mRightPlayer; // 0x1c4
    int unk1c8;
    DataArray *mGoodTitles; // 0x1cc
    DataArray *mBadTitles; // 0x1d0
    std::vector<int> unk1d4;
    Vector2DESmoother mFrameSmoothers[6]; // 0x1e0
    Difficulty mDifficulty; // 0x2d0
    Playlist *mPlaylist; // 0x2d4
    bool mIsPlaylistShuffled; // 0x2d8
    bool mUseFullLengthSongs; // 0x2d9
    int unk2dc;
    bool mPerSongDifficulty; // 0x2e0
    bool mCustomParty; // 0x2e1
    bool mUsingPerSongOptions; // 0x2e2
    float unk2e4;
    float unk2e8;
    float unk2ec;
    float mSixStarBonus; // 0x2f0
    SetPartyOptionsJob *mSetPartyOptionsJob; // 0x2f4
    GetPartyOptionsJob *mGetPartyOptionsJob; // 0x2f8
    GetPartySongQueueJob *mGetPartySongQueueJob; // 0x2fc
    AddSongToPartySongQueueJob *mAddSongToPartySongQueueJob; // 0x300
    DeleteSongFromPartySongQueueJob *mDeleteSongFromPartySongQueueJob; // 0x304
    std::list<SongQueueRow> unk308;
    int mCurrSyncedSongID; // 0x310
    bool unk314;
    DateTime unk315;
    DateTime unk31b;
    DataArray *unk324; // 0x324 - intensity sequences
    DataArray *unk328; // 0x328 - bucket sequences
    DataArray *unk32c;
    std::vector<ConfigHistory> mCfgHistories; // 0x330
};

extern PartyModeMgr *ThePartyModeMgr;
