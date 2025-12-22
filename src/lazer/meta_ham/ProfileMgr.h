#pragma once
#include "meta/FixedSizeSaveableStream.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "meta_ham/HamProfile.h"
#include "game/HamUser.h"
#include "os/PlatformMgr.h"
#include "rndobj/Overlay.h"
#include "utl/Symbol.h"

enum LagContext {
    kGame = 0,
    kVCal = 1,
    kACal = 2,
    kPractice90 = 3,
    kPractice80 = 4,
    kPractice70 = 5,
    kPractice60 = 6,
    kNumLagContexts = 7
};

class ProfileMgr : public Hmx::Object {
public:
    ProfileMgr();
    virtual DataNode Handle(DataArray *, bool);

    HamProfile *GetProfile(const LocalUser *);
    HamProfile *GetSaveData(const HamUser *);
    void PurgeOldData();
    bool NeedsUpload();
    HamProfile *GetProfileFromPad(int) const;
    int GetSavableProfileCount() const;
    HamProfile *GetFirstSavableProfile() const;
    float SliderIxToDb(int) const;
    bool GlobalOptionsNeedsSave();
    bool UnlockAllSongs();
    void RelockSongs();
    int GetFxVolume() const;
    int GetMusicVolume() const;
    int GetCrowdVolume() const;
    float GetMusicVolumeDb() const;
    float GetFxVolumeDb() const;
    float GetCrowdVolumeDb() const;
    bool GetShowVoiceTip() const;
    float GetSyncOffset(int) const;
    float GetPadExtraLag(int, LagContext) const;
    void SetMusicVolume(int);
    void PushAllOptions();
    void SetFxVolume(int);
    void SetCrowdVolume(int);
    void SetBassBoost(bool);
    void SetDolby(bool);
    void ToggleDisablePhotos();
    void ToggleNoFlashcards();
    void ToggleDisableVoice();
    void ToggleShowVoiceTip();
    void ToggleDisableFreestyle();
    void SetOverscan(bool);
    void SetMono(bool);
    void SetSyncPresetIx(int);
    void SetSyncOffset(float);
    void SetSongToTaskMgrMs(float);
    void SetExcessAudioLag(float);
    void SetExcessVideoLag(float);
    bool IsAutosaveEnabled(HamProfile *) const;
    bool HasSeenTutorial(int);
    void MarkTutorialSeen(int);
    HamProfile *GetActiveProfile(bool) const;
    HamProfile *GetNonActiveProfile() const;
    bool IsAnyProfileSignedIntoLive() const;
    bool IsContentUnlocked(Symbol) const;
    bool IsDifficultyUnlocked(Symbol, Symbol) const;
    int GetNumValidProfiles() const;
    void HandlePlayerNameChange();
    void EnableForceSpeechLanguageSupport();
    void EnableFitnessForActiveProfiles();
    bool HasFinishedCampaign() const;
    void HandleSwagJacked();
    void ToggleOverlay();
    float GetSyncOffsetRaw() const;
    float GetSongToTaskMgrMsRaw() const;
    float GetExcessAudioLag() const;
    float GetExcessVideoLag() const;
    void SetSongToTaskMgrMsRaw(float);
    void Init();
    void InitSliders();
    void SetVenuePreference(Symbol);
    bool GetDisableVoiceCommander() const;
    bool GetDisableVoicePause() const;
    bool GetDisableVoicePractice() const;
    std::vector<HamProfile *> GetAll();
    std::vector<HamProfile *> GetSignedIn();
    std::vector<HamProfile *> GetSignedInProfiles();
    int GetSliderStepCount() const;
    float GetSongToTaskMgrMs(LagContext) const;
    bool IsUnlockableContent(Symbol) const;
    bool HasUnsavedDataForPad(int);
    void CheckForServerCrewUnlock();
    void SetGlobalOptionsSaveState(ProfileSaveState);
    void SaveGlobalOptions(FixedSizeSaveableStream &);
    bool HasActiveProfile(bool) const;
    std::vector<HamProfile *> GetNewlySignedIn();
    std::vector<HamProfile *> GetShouldAutosave();
    void HandleProfileSaveComplete();
    void HandleProfileLoadComplete();
    void UpdateUsingFitnessState();
    bool HasActiveProfileWithInvalidSaveData() const;
    bool HasAnyEraSongBeenPlayed(Symbol) const;
    void Poll();

    bool GetBassBoost() const { return mBassBoost; }
    bool GetDolby() const { return mDolby; }
    Symbol GetVenuePreference() const { return mVenuePreference; }
    bool NoFlashcards() const { return mNoFlashcards; }
    bool GetAllUnlocked() { return mAllUnlocked; }

private:
    void UpdateFriendsList();

    DataNode OnMsg(const SigninChangedMsg &);

protected:
    float mPlatformAudioLatency; // 0x2c
    float mPlatformVideoLatency; // 0x30
    float unk34;
    float unk38;
    int unk3c;
    ProfileSaveState mGlobalOptionsSaveState; // 0x40
    bool mGlobalOptionsDirty; // 0x44
    bool unk45;
    int mTutorialsSeen; // 0x48
    int unk4c;
    int mMusicVolume; // 0x50
    int mFxVolume; // 0x54
    int mCrowdVolume; // 0x58
    bool mMono; // 0x5c
    float mSyncOffset; // 0x60
    float mSongToTaskMgrMs; // 0x64
    bool mBassBoost; // 0x68
    bool mDolby; // 0x69
    bool unk6a; // 0x6a
    int mSyncPresetIx; // 0x6c
    bool mOverscan; // 0x70
    bool mDisablePhotos; // 0x71
    bool mNoFlashcards; // 0x72
    bool mDisableVoice; // 0x73
    bool mDisableVoiceCommander; // 0x74
    bool mDisableVoicePause; // 0x75
    bool mDisableVoicePractice; // 0x76
    bool mShowVoiceTip; // 0x77
    bool unk78; // 0x78
    bool mDisableFreestyle; // 0x79
    Symbol mVenuePreference; // 0x7c
    int unk80;
    int unk84;
    DataArray *mSliderConfig; // 0x88
    DataArray *mVoiceChatSliderConfig; // 0x8c
    std::vector<HamProfile *> unk90;
    HamProfile *mCriticalProfile; // 0x9c
    bool mAllUnlocked; // 0xa0
    void *mProfileSaveBuffer; // 0xa4
    bool unka8;
    bool unka9;
    RndOverlay *mProfilesOverlay; // 0xac
    bool unkb0;
    Timer unkb8;
};

extern ProfileMgr TheProfileMgr;

DECLARE_MESSAGE(ProfileChangedMsg, "profile_changed_msg")
ProfileChangedMsg(Profile *p) : Message(Type(), p) {}
END_MESSAGE
