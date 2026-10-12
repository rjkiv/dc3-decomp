#pragma once
#include "hamobj/Difficulty.h"
#include "math/DoubleExponentialSmoother.h"
#include "math/Vec.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/Playlist.h"
#include "net_ham/PartyModeJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/DateTime.h"
#include "os/PlatformMgr.h"
#include "utl/MemMgr.h"
#include "utl/PseudoRandomPicker.h"

class PartyModeARObject {
public:
    PartyModeARObject(DataArray *a) : mData(a) {}
    const char *GetTexPath();

    MEM_OVERLOAD(PartyModeARObject, 0x4C);

private:
    Vector2 mOffset; // 0x0
    const DataArray *mData; // 0x8
};

class PartyModePlayer {
public:
    PartyModePlayer(PartyModeARObject *);
    ~PartyModePlayer();

    void PushTitle(Symbol);
    int GetIndex() const { return mIndex; }
    void IncScore(int score) { mScore += score; }
    void SetFramePos(float x, float y) {
        mFrameXPos = x;
        mFrameYPos = y;
    }
    void SetFrameScale(float scale) { mFrameXYScale = scale; }
    const char *GetTexPath() { return mARObject->GetTexPath(); }
    int GetPhotoIndex() const { return mPhotoIndex; }
    void SetIndex(int idx) { mIndex = idx; }
    void SetCharacterName(Symbol name) { mCharacterName = name; }
    void SetPhotoIndex(int idx) { mPhotoIndex = idx; }

private:
    PartyModeARObject *mARObject; // 0x0
    Symbol mCharacterName; // 0x4
    std::list<Symbol> mTitleTokens; // 0x8
    DataArray *mTitleTokensArray; // 0x10
    int mScore; // 0x14
    int mRank; // 0x18
    int mIndex; // 0x1c
    float mFrameXPos; // 0x20
    float mFrameYPos; // 0x4
    float mFrameXYScale; // 0x28
    int mPhotoIndex; // 0x2c
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
        SubMode() : playersArray(0) {}
        ~SubMode() {
            if (playersArray) {
                playersArray->Release();
            }
        }

        MEM_OVERLOAD(SubMode, 0xE3);

        Symbol mode; // 0x0 - mode name
        Symbol microgame; // 0x4 - sub mode name
        Symbol song; // 0x8 - song shortname
        int songID; // 0xc
        int playerFlags; // 0x10
        int numPlayers; // 0x14
        int timeTilInMS;
        std::vector<int> players;
        DataArray *playersArray; // 0x28
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

    Difficulty GetDifficulty() { return mDifficulty; }
    void SetDifficulty(Difficulty d) { mDifficulty = d; }
    bool IsUsingFullLengthSongs() { return mUseFullLengthSongs; }
    void UseFullLengthSongs(bool b) { mUseFullLengthSongs = b; }
    Playlist *GetPlaylist() const { return mPlaylist; }
    const DateTime &GetCurrentPartyTime() const { return mCurrPartyTime; }

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
    void ReadPartySongQueue();

    Symbol GetCurrEventDisplayName() { return GetCurrEventName(); }
    bool IsUsingPlaylist() const { return mPlaylist; }

    DataNode OnGetSmoothedFramePos(const DataArray *);
    DataNode OnSetSongAndDefaults(DataArray *);
    DataNode OnStableSong();
    DataNode OnStableMode();
    DataNode OnMsg(const RCJobCompleteMsg &);
    DataNode OnMsg(const SmartGlassMsg &);

    DataArray *mData; // 0x30
    bool mPlaytestMode; // 0x34
    DataArray *mPlaytestData; // 0x38
    DataArray *mPlaytestEvents; // 0x3c
    bool unk40; // 0x40 - not in pdb
    SubMode *mCurrEvent; // 0x44
    int mRoundsPlayed; // 0x48
    int mRoundsBetweenShowdowns; // 0x4c
    int mRoundsUntilShowdown; // 0x50
    bool mIsShowdownTime; // 0x54
    const DataArray *mARObjectsData; // 0x58
    std::vector<PartyModePlayer *> mPlayers; // 0x5c
    std::vector<PartyModePlayer *> mPlayersTeam1; // 0x68
    std::vector<PartyModePlayer *> mPlayersTeam2; // 0x74
    float mLeftTeamPrevScore; // 0x80
    float mLeftTeamScore; // 0x84
    float mRightTeamPrevScore; // 0x88
    float mRightTeamScore; // 0x8c
    float mLeftTeamStarBonus; // 0x90
    float mRightTeamStarBonus; // 0x94
    DataArray *mEventScoringData; // 0x98
    int mWinningSide; // 0x9c
    int mJustWonSide; // 0xa0
    std::vector<int> mRandomNextPlayerIndices; // 0xa4
    std::vector<int> mRandomARObjectIndices; // 0xb0
    PseudoRandomPicker<int> mPlayerPicker; // 0xbc
    PseudoRandomPicker<int> mPlayerTeam1Picker; // 0xd0
    PseudoRandomPicker<int> mPlayerTeam2Picker; // 0xe4
    PseudoRandomPicker<Symbol> mSongPicker; // 0xf8
    PseudoRandomPicker<Symbol> mModePicker; // 0x10c
    PseudoRandomPicker<Symbol> mMicrogamePicker; // 0x120
    PseudoRandomPicker<Symbol> mGoodTitlePicker; // 0x134
    PseudoRandomPicker<Symbol> mBadTitlePicker; // 0x148
    // indexed by dj intensity rank
    PseudoRandomPicker<Symbol> mSongPickers[4]; // 0x15c
    std::vector<Symbol> mCharacters; // 0x1ac
    Symbol mLeftTeamCrew; // 0x1b8
    Symbol mRightTeamCrew; // 0x1bc
    PartyModePlayer *mLeftPlayer; // 0x1c0
    PartyModePlayer *mRightPlayer; // 0x1c4
    int mLastTeamPickedFrom; // 0x1c8
    DataArray *mGoodTitles; // 0x1cc
    DataArray *mBadTitles; // 0x1d0
    std::vector<int> mValidSongIDs; // 0x1d4
    Vector2DESmoother mFrameMotionSmoother[6]; // 0x1e0
    Difficulty mDifficulty; // 0x2d0 - int in PDB but cmon... Difficulty is right here
    Playlist *mPlaylist; // 0x2d4
    bool mPlaylistIsShuffled; // 0x2d8
    bool mUseFullLengthSongs; // 0x2d9
    int mIncludedModeFlags; // 0x2dc
    bool mPerSongDifficulty; // 0x2e0
    bool mCustomParty; // 0x2e1
    bool mUsePerSongOptions; // 0x2e2
    float mLeftTeamPrevMeterPct; // 0x2e4
    float mRightTeamPrevMeterPct; // 0x2e8
    float mMaxPossibleEventPoints; // 0x2ec
    float mMaxPossibleStarBonus; // 0x2f0
    RCJob *mPartyJobs[5]; // 0x2f4
    // SetPartyOptionsJob [0] - 0x2f4
    // GetPartyOptionsJob [1] - 0x2f8
    // GetPartySongQueueJob [2] - 0x2fc
    // AddSongToPartySongQueueJob [3]-  0x300
    // DeleteSongFromPartySongQueueJob [4] - 0x304
    std::list<SongQueueRow> mPartySongQueue; // 0x308
    int mCurrSyncedSongID; // 0x310
    bool mAddingSongToRCPartySongQueue; // 0x314
    DateTime mCurrPartyTime; // 0x315
    DateTime mPartyStartTime; // 0x31b
    DataArray *mDjIntensitySequence; // 0x324 - intensity sequences
    DataArray *mModeBucketSequence; // 0x328 - bucket sequences
    DataArray *mPlayerSequence;
    std::vector<ConfigHistory> mCfgHistories; // 0x330 - not in PDB
};

extern PartyModeMgr *ThePartyModeMgr;
