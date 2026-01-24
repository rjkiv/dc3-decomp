#include "meta_ham/ProfileMgr.h"
#include "HamProfile.h"
#include "HamUI.h"
#include "ProfileMgr.h"
#include "flow/PropertyEventProvider.h"
#include "game/Game.h"
#include "gesture/SpeechMgr.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Utl.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta/MemcardMgr.h"
#include "meta/Profile.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/FitnessGoalMgr.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/MetaPanel.h"
#include "meta_ham/ShellInput.h"
#include "meta_ham/SkeletonChooser.h"
#include "meta_ham/UIEventMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "os/User.h"
#include "rndobj/Overlay.h"
#include "rndobj/Rnd.h"
#include "synth/FxSend.h"
#include "synth/Synth.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "game/HamUser.h"

ProfileMgr TheProfileMgr;

ProfileMgr::ProfileMgr()
    : mPlatformAudioLatency(5), mPlatformVideoLatency(22), unk34(22), unk38(50),
      mGlobalOptionsSaveState(kMetaProfileUnloaded), mGlobalOptionsDirty(0), unk45(0),
      mTutorialsSeen(0), unk4c(0), mMusicVolume(4), mFxVolume(4), mCrowdVolume(4),
      mMono(0), mSyncOffset(0), mSongToTaskMgrMs(0), mBassBoost(0), mDolby(0), unk6a(0),
      mSyncPresetIx(0), mOverscan(0), mDisablePhotos(0), mNoFlashcards(0),
      mDisableVoice(0), mDisableVoiceCommander(0), mDisableVoicePause(0),
      mDisableVoicePractice(0), mShowVoiceTip(1), unk78(0), mDisableFreestyle(0),
      mVenuePreference(gNullStr), unk80(0), unk84(0), mCriticalProfile(0),
      mAllUnlocked(0), mProfileSaveBuffer(0), unka8(0), unka9(0), mProfilesOverlay(0),
      unkb0(0) {
    mSyncOffset = -mPlatformVideoLatency;
}

BEGIN_HANDLERS(ProfileMgr)
    HANDLE_EXPR(get_profile, GetProfile(_msg->Obj<HamUser>(2)))
    HANDLE_EXPR(get_save_data, GetSaveData(_msg->Obj<HamUser>(2)))
    HANDLE_ACTION(purge_old_data, PurgeOldData())
    HANDLE_EXPR(needs_upload, NeedsUpload())
    HANDLE_EXPR(get_profile_from_pad, GetProfileFromPad(_msg->Int(2)))
    HANDLE_EXPR(get_savable_profile_count, GetSavableProfileCount())
    HANDLE_EXPR(get_first_savable_profile, GetFirstSavableProfile())
    HANDLE_EXPR(unlock_all_songs, UnlockAllSongs())
    HANDLE_ACTION(relock_songs, RelockSongs())
    HANDLE_EXPR(get_all_unlocked, GetAllUnlocked())
    HANDLE_EXPR(global_options_needs_save, GlobalOptionsNeedsSave())
    HANDLE_EXPR(get_music_volume, GetMusicVolume())
    HANDLE_EXPR(get_music_volume_db, GetMusicVolumeDb())
    HANDLE_EXPR(get_fx_volume, GetFxVolume())
    HANDLE_EXPR(get_fx_volume_db, GetFxVolumeDb())
    HANDLE_EXPR(get_crowd_volume, GetCrowdVolume())
    HANDLE_EXPR(get_crowd_volume_db, GetCrowdVolumeDb())
    HANDLE_EXPR(get_bass_boost, GetBassBoost())
    HANDLE_EXPR(get_dolby, GetDolby())
    HANDLE_EXPR(get_venue_preference, mVenuePreference)
    HANDLE_EXPR(get_disable_photos, mDisablePhotos)
    HANDLE_EXPR(get_disable_voice, mDisableVoice)
    HANDLE_EXPR(get_disable_voice_commander, GetDisableVoiceCommander())
    HANDLE_EXPR(get_disable_voice_pause, GetDisableVoicePause())
    HANDLE_EXPR(get_disable_voice_practice, GetDisableVoicePractice())
    HANDLE_EXPR(get_show_voice_tip, GetShowVoiceTip())
    HANDLE_EXPR(get_disable_freestyle, mDisableFreestyle)
    HANDLE_EXPR(get_no_flashcards, mNoFlashcards)
    HANDLE_EXPR(get_overscan, mOverscan)
    HANDLE_EXPR(get_mono, mMono)
    HANDLE_EXPR(get_sync_preset_ix, mSyncPresetIx)
    HANDLE_EXPR(get_sync_offset_raw, GetSyncOffsetRaw())
    HANDLE_EXPR(get_sync_offset, GetSyncOffset(_msg->Int(2)))
    HANDLE_EXPR(get_song_to_taskmgr_ms, mSongToTaskMgrMs - unk34)
    HANDLE_EXPR(get_song_to_taskmgr_ms_raw, GetSongToTaskMgrMsRaw())
    HANDLE_EXPR(get_excess_audio_lag, GetExcessAudioLag())
    HANDLE_EXPR(get_excess_video_lag, GetExcessVideoLag())
    HANDLE_EXPR(get_pad_extra_lag, GetPadExtraLag(_msg->Int(2), kGame))
    HANDLE_ACTION(set_music_volume, SetMusicVolume(_msg->Int(2)))
    HANDLE_ACTION(set_fx_volume, SetFxVolume(_msg->Int(2)))
    HANDLE_ACTION(set_crowd_volume, SetCrowdVolume(_msg->Int(2)))
    HANDLE_ACTION(set_bass_boost, SetBassBoost(_msg->Int(2)))
    HANDLE_ACTION(set_dolby, SetDolby(_msg->Int(2)))
    HANDLE_ACTION(toggle_disable_photos, ToggleDisablePhotos())
    HANDLE_ACTION(toggle_no_flashcards, ToggleNoFlashcards())
    HANDLE_ACTION(toggle_disable_voice, ToggleDisableVoice())
    HANDLE_ACTION(toggle_show_voice_tip, ToggleShowVoiceTip())
    HANDLE_ACTION(toggle_disable_freestyle, ToggleDisableFreestyle())
    HANDLE_ACTION(set_overscan, SetOverscan(_msg->Int(2)))
    HANDLE_ACTION(set_mono, SetMono(_msg->Int(2)))
    HANDLE_ACTION(set_sync_preset_ix, SetSyncPresetIx(_msg->Int(2)))
    HANDLE_ACTION(set_sync_offset, SetSyncOffset(_msg->Float(2)))
    HANDLE_ACTION(set_song_to_taskmgr_ms, SetSongToTaskMgrMs(_msg->Float(2)))
    HANDLE_ACTION(set_excess_audio_lag, SetExcessAudioLag(_msg->Float(2)))
    HANDLE_ACTION(set_excess_video_lag, SetExcessVideoLag(_msg->Float(2)))
    HANDLE_EXPR(is_autosave_enabled, IsAutosaveEnabled(_msg->Obj<HamProfile>(2)))
    HANDLE_EXPR(has_seen_tutorial, HasSeenTutorial(_msg->Int(2)))
    HANDLE_ACTION(mark_tutorial_seen, MarkTutorialSeen(_msg->Int(2)))
    HANDLE_EXPR(get_active_profile, GetActiveProfile(true))
    HANDLE_EXPR(has_active_profile, GetActiveProfile(true) != nullptr)
    HANDLE_EXPR(get_non_active_profile, GetNonActiveProfile())
    HANDLE_EXPR(is_any_profile_signed_into_live, IsAnyProfileSignedIntoLive())
    HANDLE_EXPR(get_active_profile_no_override, GetActiveProfile(false))
    HANDLE_EXPR(has_active_profile_no_override, GetActiveProfile(false) != nullptr)
    HANDLE_EXPR(is_content_unlocked, IsContentUnlocked(_msg->ForceSym(2)))
    HANDLE_EXPR(is_difficulty_unlocked, IsDifficultyUnlocked(_msg->Sym(2), _msg->Sym(3)))
    HANDLE_ACTION(set_critical_profile, mCriticalProfile = _msg->Obj<HamProfile>(2))
    HANDLE_ACTION(clear_critical_profile, mCriticalProfile = nullptr)
    HANDLE_EXPR(get_num_valid_profiles, GetNumValidProfiles())
    HANDLE_ACTION(pose_found, _msg->Int(2))
    HANDLE_ACTION(on_player_name_change, HandlePlayerNameChange())
    HANDLE_ACTION(enable_force_speech_language_support, EnableForceSpeechLanguageSupport())
    HANDLE_ACTION(enable_fitness_for_active_profiles, EnableFitnessForActiveProfiles())
    HANDLE_EXPR(is_voice_commander_suboptimal, !TheSpeechMgr->SpeechSupported())
    HANDLE_EXPR(has_finished_campaign, HasFinishedCampaign())
    HANDLE_ACTION(swag_jacked, HandleSwagJacked())
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_ACTION(toggle_overlay, ToggleOverlay())
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool ProfileMgr::GetDisableVoiceCommander() const { return mDisableVoiceCommander; }
bool ProfileMgr::GetDisableVoicePause() const { return mDisableVoicePause; }
bool ProfileMgr::GetDisableVoicePractice() const { return mDisableVoicePractice; }

bool ProfileMgr::UnlockAllSongs() {
    bool old = mAllUnlocked;
    mAllUnlocked = true;
    return old == 0;
}

void ProfileMgr::RelockSongs() { mAllUnlocked = false; }

bool ProfileMgr::GlobalOptionsNeedsSave() {
    if (mGlobalOptionsSaveState != kMetaProfileLoaded)
        return false;
    else
        return mGlobalOptionsDirty;
}

int ProfileMgr::GetMusicVolume() const { return mMusicVolume; }
int ProfileMgr::GetFxVolume() const { return unk6a ? 0 : mFxVolume; }
int ProfileMgr::GetCrowdVolume() const { return mCrowdVolume; }

float ProfileMgr::GetMusicVolumeDb() const { return SliderIxToDb(GetMusicVolume()); }
float ProfileMgr::GetFxVolumeDb() const { return SliderIxToDb(GetFxVolume()); }
float ProfileMgr::GetCrowdVolumeDb() const { return SliderIxToDb(GetCrowdVolume()); }
bool ProfileMgr::GetShowVoiceTip() const { return !mDisableVoice && mShowVoiceTip; }

void ProfileMgr::SetFxVolume(int vol) {
    mFxVolume = vol;
    mGlobalOptionsDirty = true;
    PushAllOptions();
}

void ProfileMgr::SetCrowdVolume(int vol) {
    mCrowdVolume = vol;
    mGlobalOptionsDirty = true;
    PushAllOptions();
}

void ProfileMgr::SetBassBoost(bool b) {
    mBassBoost = b;
    mGlobalOptionsDirty = true;
    PushAllOptions();
}

void ProfileMgr::SetDolby(bool b) {
    mDolby = b;
    mGlobalOptionsDirty = true;
    PushAllOptions();
}

void ProfileMgr::ToggleDisablePhotos() {
    mGlobalOptionsDirty = true;
    mDisablePhotos = !mDisablePhotos;
}

void ProfileMgr::ToggleNoFlashcards() {
    mGlobalOptionsDirty = true;
    mNoFlashcards = !mNoFlashcards;
}

void ProfileMgr::ToggleDisableVoice() {
    mGlobalOptionsDirty = true;
    mDisableVoice = !mDisableVoice;
    mDisableVoiceCommander = !mDisableVoiceCommander;
    mDisableVoicePause = !mDisableVoicePause;
    mDisableVoicePractice = !mDisableVoicePractice;
}

void ProfileMgr::ToggleShowVoiceTip() {
    mGlobalOptionsDirty = true;
    mShowVoiceTip = !mShowVoiceTip;
}

void ProfileMgr::ToggleDisableFreestyle() {
    mGlobalOptionsDirty = true;
    mDisableFreestyle = !mDisableFreestyle;
}

void ProfileMgr::SetMono(bool mono) {
    mGlobalOptionsDirty = true;
    mMono = mono;
    PushAllOptions();
}

void ProfileMgr::SetSyncPresetIx(int idx) {
    mSyncPresetIx = idx;
    mGlobalOptionsDirty = true;
}

void ProfileMgr::SetSyncOffset(float offset) {
    if (mSyncOffset != offset) {
        mSyncOffset = offset;
        mGlobalOptionsDirty = true;
        PushAllOptions();
    }
}

void ProfileMgr::SetSongToTaskMgrMs(float ms) {
    if (mSongToTaskMgrMs != ms) {
        mSongToTaskMgrMs = ms;
        mGlobalOptionsDirty = true;
    }
}

void ProfileMgr::SetExcessAudioLag(float lag) {
    SetSongToTaskMgrMsRaw(-(lag + mPlatformAudioLatency + GetSyncOffsetRaw()));
}

void ProfileMgr::SetSongToTaskMgrMsRaw(float ms) {
    if (mSongToTaskMgrMs != ms) {
        mSongToTaskMgrMs = ms;
        mGlobalOptionsDirty = true;
    }
}

bool ProfileMgr::IsAutosaveEnabled(HamProfile *p) const {
    return p && p->GetSaveState() != kMetaProfileError;
}

bool ProfileMgr::HasSeenTutorial(int tutorial) {
    return mTutorialsSeen & tutorial
        || TheProfileMgr.mGlobalOptionsSaveState != kMetaProfileLoaded;
}

void ProfileMgr::MarkTutorialSeen(int tutorial) {
    mGlobalOptionsDirty = true;
    mTutorialsSeen |= tutorial;
}

void ProfileMgr::EnableForceSpeechLanguageSupport() {
    mDisableVoiceCommander = false;
    mDisableVoice = false;
    mDisableVoicePause = false;
    mDisableVoicePractice = false;
    unk78 = true;
    mGlobalOptionsDirty = true;
}

void ProfileMgr::ToggleOverlay() {
    if (mProfilesOverlay) {
        mProfilesOverlay->SetShowing(!mProfilesOverlay->Showing());
    }
}

float ProfileMgr::GetSyncOffsetRaw() const { return mSyncOffset; }
float ProfileMgr::GetSongToTaskMgrMsRaw() const { return mSongToTaskMgrMs; }
float ProfileMgr::GetExcessAudioLag() const {
    return -(mPlatformAudioLatency + GetSongToTaskMgrMsRaw() + GetSyncOffsetRaw());
}
float ProfileMgr::GetExcessVideoLag() const {
    return -(mPlatformVideoLatency + GetSyncOffsetRaw());
}

void ProfileMgr::SetVenuePreference(Symbol venue) { mVenuePreference = venue; }

void ProfileMgr::Init() {
    for (int i = 0; i < 4; i++) {
        unk90.push_back(new HamProfile(i));
    }
    SetName("profile_mgr", ObjectDir::Main());
    ThePlatformMgr.AddSink(this, SigninChangedMsg::Type());
    if (TheGameData) {
        TheGameData->AddSink(this, "pose_found");
        HamPlayerData *pPlayer0 = TheGameData->Player(0);
        MILO_ASSERT(pPlayer0, 0xA0);
        HamPlayerData *pPlayer1 = TheGameData->Player(1);
        MILO_ASSERT(pPlayer1, 0xA2);
        PropertyEventProvider *pProvider0 = pPlayer0->Provider();
        MILO_ASSERT(pProvider0, 0xA5);
        PropertyEventProvider *pProvider1 = pPlayer1->Provider();
        MILO_ASSERT(pProvider1, 0xA7);
        static Symbol on_player_name_change("on_player_name_change");
        pProvider0->AddSink(this, on_player_name_change, gNullStr, kHandle, false);
        pProvider1->AddSink(this, on_player_name_change, gNullStr, kHandle, false);
        static Symbol swag_jacked("swag_jacked");
        pProvider0->AddSink(this, swag_jacked, gNullStr, kHandle, false);
        pProvider1->AddSink(this, swag_jacked, gNullStr, kHandle, false);
    }
    InitSliders();
    static Symbol eng("eng");
    if (ThePlatformMgr.GetRegion() == kRegionNA && SystemLanguage() == eng) {
        unk4c = 0;
    } else {
        unk4c = 1;
    }
    MILO_ASSERT(mProfileSaveBuffer == NULL, 0xBC);
    int size = FixedSizeSaveableStream::GetSymbolTableSize(0x5C) + 8;
    size += HamProfile::SaveSize(0x5C);
    mProfileSaveBuffer = MemAlloc(size, __FILE__, 0xBE, "ProfileSaveBuffer");
    TheMemcardMgr.SetProfileSaveBuffer(mProfileSaveBuffer, size);
    static Symbol defaultSym("default");
    mVenuePreference = defaultSym;
    MILO_ASSERT(TheSynth, 0xCA);
    mDolby = TheSynth->IsUsingDolby();
    if (!TheSpeechMgr->SpeechSupported()) {
        mDisableVoiceCommander = true;
        mDisableVoice = true;
        mDisableVoicePause = true;
        mDisableVoicePractice = true;
        mShowVoiceTip = false;
    }
    mProfilesOverlay = RndOverlay::Find("profiles", false);
}

std::vector<HamProfile *> ProfileMgr::GetAll() {
    std::vector<HamProfile *> all;
    FOREACH (it, unk90) {
        all.push_back(*it);
    }
    return all;
}

std::vector<HamProfile *> ProfileMgr::GetSignedIn() {
    std::vector<HamProfile *> profiles;
    FOREACH (it, unk90) {
        if (ThePlatformMgr.IsSignedIn((*it)->GetPadNum())) {
            profiles.push_back(*it);
        }
    }
    return profiles;
}

std::vector<HamProfile *> ProfileMgr::GetSignedInProfiles() {
    std::vector<HamProfile *> profiles;
    FOREACH (it, unk90) {
        if (ThePlatformMgr.IsSignedIn((*it)->GetPadNum())) {
            profiles.push_back(*it);
        }
    }
    return profiles;
}

float ProfileMgr::SliderIxToDb(int ixVol) const {
    MILO_ASSERT(mSliderConfig, 0x34B);
    MILO_ASSERT(0 <= ixVol && ixVol < mSliderConfig->Size() - 1, 0x34C);
    return mSliderConfig->Float(ixVol + 1);
}

int ProfileMgr::GetSliderStepCount() const {
    MILO_ASSERT(mSliderConfig, 0x352);
    return mSliderConfig->Size() - 1;
}

void ProfileMgr::InitSliders() {
    if (!mSliderConfig) {
        mSliderConfig = SystemConfig("sound", "slider");
    }
    if (!mVoiceChatSliderConfig) {
        mVoiceChatSliderConfig = SystemConfig("sound", "slider_voicechat");
    }
}

void ProfileMgr::SetOverscan(bool overscan) {
    if (overscan != mOverscan) {
        mOverscan = overscan;
        TheRnd.SetShrinkToSafeArea(!mOverscan);
        mGlobalOptionsDirty = true;
    }
}

float ProfileMgr::GetSongToTaskMgrMs(LagContext lc) const {
    switch (lc) {
    case kPractice90:
        return mSongToTaskMgrMs - unk34 - 0x46;
    case kPractice80:
    case kPractice70:
    default:
        break;
    }
    return 0;
}

// bool ProfileMgr::IsUnlockableContent(Symbol s) const {
//   return TheAccomplishmentMgr.IsUnlockableAsset(s);
// }

HamProfile *ProfileMgr::GetProfileFromPad(int pad) const {
    HamProfile *ret = nullptr;
    FOREACH (it, unk90) {
        if ((*it)->GetPadNum() == pad) {
            ret = *it;
            break;
        }
    }
    return ret;
}

int ProfileMgr::GetSavableProfileCount() const {
    int i = 0;
    FOREACH (it, unk90) {
        HamProfile *pProfile = *it;
        MILO_ASSERT(pProfile, 0x191);
        if (pProfile->HasValidSaveData())
            i++;
    }
    return i;
}

HamProfile *ProfileMgr::GetFirstSavableProfile() const {
    FOREACH (it, unk90) {
        HamProfile *pProfile = *it;
        MILO_ASSERT(pProfile, 0x1A1);
        if (pProfile->HasValidSaveData())
            return pProfile;
    }
    return nullptr;
}

HamProfile *ProfileMgr::GetProfile(const LocalUser *user) {
    if (user && user->GetPadNum() != -1) {
        return GetProfileFromPad(user->GetPadNum());
    } else {
        return nullptr;
    }
}

bool ProfileMgr::HasUnsavedDataForPad(int padNum) {
    MILO_ASSERT(padNum != -1, 0x220);
    HamProfile *profile = GetProfileFromPad(padNum);
    return ThePlatformMgr.IsSignedIn(padNum)
        && profile->GetSaveState() == kMetaProfileLoaded && profile->IsUnsaved();
    return 0;
}

HamProfile *ProfileMgr::GetSaveData(const HamUser *user) {
    if (user && user->IsLocal() && user->CanSaveData()) {
        HamProfile *profile = GetProfileFromPad(user->GetPadNum());
        MILO_ASSERT(profile, 0x1ED);
        return profile;
    } else {
        return nullptr;
    }
}

bool ProfileMgr::NeedsUpload() {
    if (!mAllUnlocked) {
        FOREACH (it, unk90) {
            if ((*it)->HasSomethingToUpload()) {
                return true;
            }
        }
    }
    return false;
}

void ProfileMgr::CheckForServerCrewUnlock() {
    for (int i = 0; i < unk90.size(); i++) {
        if (unk90[i]->HasValidSaveData()) {
            unk90[i]->CheckForIconManUnlock();
            unk90[i]->CheckForNinjaUnlock();
        }
    }
}

void ProfileMgr::SetGlobalOptionsSaveState(ProfileSaveState state) {
    MILO_ASSERT(mGlobalOptionsSaveState != kMetaProfileUnchanged, 0x2A4);
    if (state != kMetaProfileUnchanged) {
        mGlobalOptionsSaveState = state;
    }
}

void ProfileMgr::SaveGlobalOptions(FixedSizeSaveableStream &fs) {
    fs << 0x1C;
    fs << mMono;
    fs << mSyncOffset;
    fs << mSongToTaskMgrMs;
    fs << mMusicVolume;
    fs << mFxVolume;
    fs << mBassBoost;
    fs << mCrowdVolume;
    fs << mDolby;
    fs << mDisablePhotos;
    fs << mNoFlashcards;
    fs << mDisableVoice;
    fs << mDisableVoiceCommander;
    fs << mDisableVoicePause;
    fs << mDisableVoicePractice;
    fs << mShowVoiceTip;
    fs << mDisableFreestyle;
    fs << mSyncPresetIx;
    fs << mOverscan;
    fs << mTutorialsSeen;
    fs << unk4c;
    fs << unk78;
    fs << (u64)unk80;
    fs << (u64)unk84;
    mGlobalOptionsDirty = false;
}

int ProfileMgr::GlobalOptionsSaveSize() { return 0x38; }

void ProfileMgr::EnableFitnessForActiveProfiles() {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x3CB);
        HamProfile *pProfile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
        if (pProfile && pProfile->HasValidSaveData()) {
            pProfile->SetFitnessMode(true);
        }
    }
}

float ProfileMgr::GetSyncOffset(int i1) const {
    float f1 = 0;
    if (i1 != -1) {
        f1 = GetPadExtraLag(i1, kGame);
    }
    return mSyncOffset + unk38 - f1;
}

bool ProfileMgr::IsAnyProfileSignedIntoLive() const {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x5A7);
        int padNum = pPlayer->PadNum();
        if (TheProfileMgr.GetProfileFromPad(padNum)
            && ThePlatformMgr.IsSignedIntoLive(padNum)) {
            return true;
        }
    }
    return false;
}

bool ProfileMgr::HasActiveProfile(bool b1) const { return GetActiveProfile(b1); }

void ProfileMgr::PurgeOldData() {
    FOREACH (it, unk90) {
        HamProfile *pProfile = *it;
        if (pProfile->GetSaveState() == kMetaProfileDelete) {
            pProfile->DeleteAll();
            pProfile->SetSaveState(kMetaProfileUnloaded);
            static ProfileChangedMsg msg(pProfile);
            msg[0] = pProfile;
            Handle(msg, false);
        }
    }
}

void ProfileMgr::PushAllOptions() {
    float musicDb = GetMusicVolumeDb();
    float crowdDb = GetCrowdVolumeDb();
    float fxDb = GetFxVolumeDb();
    if (TheGame) {
        TheGame->SetBackgroundVolume(musicDb);
        if (TheGame) {
            TheGame->SetBackgroundVolume(musicDb);
        }
    }
    Fader *bFade = TheSynth->Find<Fader>("background_music_level.fade", false);
    if (bFade) {
        bFade->SetVolume(musicDb);
    }
    Fader *cFade = TheSynth->Find<Fader>("crowd_level.fade", false);
    if (cFade) {
        cFade->SetVolume(crowdDb);
    }
    Fader *sFade = TheSynth->Find<Fader>("sfx_level.fade", false);
    if (sFade) {
        sFade->SetVolume(fxDb);
    }
    Fader *iFade = TheSynth->Find<Fader>("instrument_level.fade", false);
    if (iFade) {
        iFade->SetVolume(musicDb);
    }
    FxSend *send = TheSynth->Find<FxSend>("bass_boost.send", false);
    if (send) {
        if (mBassBoost) {
            send->SetProperty("wet_gain", 0.0f);
            send->SetProperty("dry_gain", kDbSilence);
        } else {
            send->SetProperty("wet_gain", kDbSilence);
            send->SetProperty("dry_gain", 0.0f);
        }
    }
    TheRnd.SetShrinkToSafeArea(!mOverscan);
    TheSynth->SetDolby(mDolby, true);
    static DataNode &n = DataVariable("crowd_audio.volume");
    n = GetCrowdVolumeDb();
    if (TheSpeechMgr) {
        if (unk78 && TheSpeechMgr->OnIsSpeechSupportable()) {
            TheSpeechMgr->ForceLanguageSupport();
        } else if (!TheSpeechMgr->SpeechSupported() && !mDisableVoice) {
            mDisableVoice = !mDisableVoice;
            mGlobalOptionsDirty = true;
            mDisableVoiceCommander = !mDisableVoiceCommander;
            mDisableVoicePause = !mDisableVoicePause;
            mDisableVoicePractice = !mDisableVoicePractice;
        }
    }
    DWORD locale = ULSystemLocale();
    DWORD language = ULSystemLanguage();
    if (TheSpeechMgr && (locale != unk80 || language != unk84)
        && TheSpeechMgr->SpeechSupported() && mDisableVoice) {
        mDisableVoice = !mDisableVoice;
        mGlobalOptionsDirty = true;
        mDisableVoiceCommander = !mDisableVoiceCommander;
        mDisableVoicePause = !mDisableVoicePause;
        mDisableVoicePractice = !mDisableVoicePractice;
    }
    unk80 = locale;
    unk84 = language;
    mGlobalOptionsDirty = true;
}

void ProfileMgr::SetMusicVolume(int step) {
    if (step >= 0 && GetSliderStepCount() > step) {
        mMusicVolume = step;
        mGlobalOptionsDirty = true;
        PushAllOptions();
    }
}

void ProfileMgr::SetExcessVideoLag(float lag) {
    float audioLag = GetExcessAudioLag();
    SetSyncOffset(-(mPlatformVideoLatency + lag));
    SetExcessAudioLag(audioLag);
}

void ProfileMgr::HandlePlayerNameChange() {
    static Symbol ui_nav_mode("ui_nav_mode");
    const DataNode *pNavPlayerNode = TheHamProvider->Property(ui_nav_mode);
    MILO_ASSERT(pNavPlayerNode, 0x6FA);
    Symbol nodeSym = pNavPlayerNode->Sym();
    static Symbol store("store");
    if (nodeSym == store) {
        HamPlayerData *pPlayer0 = TheGameData->Player(0);
        MILO_ASSERT(pPlayer0, 0x704);
        HamProfile *pProfile = GetProfileFromPad(pPlayer0->PadNum());
        if (mCriticalProfile) {
            if (mCriticalProfile->HasValidSaveData() && pProfile != mCriticalProfile) {
                static Symbol store_user_change("store_user_change");
                static Message init("init");
                // TheUIEventMgr.TriggerEvent
            }
        }
    }
}

std::vector<HamProfile *> ProfileMgr::GetNewlySignedIn() {
    std::vector<HamProfile *> profiles;
    FOREACH (it, unk90) {
        HamProfile *pProfile = *it;
        int padNum = pProfile->GetPadNum();
        if (ThePlatformMgr.IsSignedIn(padNum) && !ThePlatformMgr.IsPadAGuest(padNum)
            && pProfile->GetSaveState() == kMetaProfileUnloaded) {
            profiles.push_back(*it);
        }
    }
    return profiles;
}

std::vector<HamProfile *> ProfileMgr::GetShouldAutosave() {
    std::vector<HamProfile *> profiles;
    FOREACH (it, unk90) {
        HamProfile *pProfile = *it;
        int padNum = pProfile->GetPadNum();
        if (ThePlatformMgr.IsSignedIn(padNum)
            && pProfile->GetSaveState() == kMetaProfileLoaded && pProfile->IsUnsaved()) {
            profiles.push_back(pProfile);
        }
    }
    return profiles;
}

void ProfileMgr::HandleProfileSaveComplete() {
    UpdateFriendsList();
    UpdateUsingFitnessState();
}

void ProfileMgr::HandleProfileLoadComplete() {
    UpdateFriendsList();
    UpdateUsingFitnessState();
    unka8 = true;
    unka9 = true;
}

int ProfileMgr::GetNumValidProfiles() const {
    int count = 0;
    std::vector<HamProfile *> profiles = TheProfileMgr.GetAll();
    FOREACH (it, profiles) {
        HamProfile *pProfile = *it;
        MILO_ASSERT(pProfile, 0x6EC);
        if (pProfile->HasValidSaveData()) {
            count++;
        }
    }
    return count;
}

void ProfileMgr::HandleSwagJacked() {
    HamProfile *pProfile = GetActiveProfile(true);
    static Symbol acc_jack_swag("acc_jack_swag");
    TheAccomplishmentMgr->EarnAccomplishmentForProfile(pProfile, acc_jack_swag, true);
}

bool ProfileMgr::HasActiveProfileWithInvalidSaveData() const {
    HamProfile *pProfile = GetActiveProfile(true);
    if (!pProfile) {
        ShellInput *pShellInput = TheHamUI.GetShellInput();
        MILO_ASSERT(pShellInput, 0x5bb);
        SkeletonChooser *pSkeletonChooser = pShellInput->mSkelChooser;
        MILO_ASSERT(pSkeletonChooser, 0x5be);
        HamPlayerData *pActivePlayer = TheGameData->Player(pSkeletonChooser->Unk3C());
        MILO_ASSERT(pActivePlayer, 0x5c2);
        HamProfile *pProfileFromPad =
            TheProfileMgr.GetProfileFromPad(pActivePlayer->PadNum());
        if (pProfileFromPad && !pProfile->HasValidSaveData())
            return true;
    }
    return false;
}

bool ProfileMgr::IsUnlockableContent(Symbol s) const {
    return TheAccomplishmentMgr->IsUnlockableAsset(s);
}

void ProfileMgr::UploadDeferredFlaunt() {
    if (!unka8)
        return;
    unka8 = false;
    TheChallenges->UploadFlauntForAll(true);
}

HamProfile *ProfileMgr::GetNonActiveProfile() const {
    HamProfile *pActiveProfile = GetActiveProfile(false);
    if (pActiveProfile) {
        for (int i = 0; i <= 1; i++) {
            HamPlayerData *pOtherPlayer = TheGameData->Player(i);
            MILO_ASSERT(pOtherPlayer, 0x595);
            HamProfile *pProfile =
                TheProfileMgr.GetProfileFromPad(pOtherPlayer->PadNum());
            if (pProfile && pProfile->HasValidSaveData() && pProfile != pActiveProfile)
                return pProfile;
        }
    }
    return nullptr;
}

bool ProfileMgr::HasFinishedCampaign() const {
    if (MetaPanel::sUnlockAll) {
        return true;
    } else {
        FOREACH (it, unk90) {
            HamProfile *profile = *it;
            MILO_ASSERT(profile, 0x6ae);
            if (profile->HasFinishedCampaign()) {
                return true;
            }
        }
    }
    return false;
}

bool ProfileMgr::HasAnyEraSongBeenPlayed(Symbol era) const {
    if (MetaPanel::sUnlockAll) {
        return true;
    } else {
        FOREACH (it, unk90) {
            HamProfile *profile = *it;
            MILO_ASSERT(profile, 0x6c3);
            if (profile->HasAnyEraSongBeenPlayed(era)) {
                return true;
            }
        }
    }
    return false;
}

bool ProfileMgr::IsDifficultyUnlocked(Symbol s1, Symbol s2) const {
    if (MetaPanel::sUnlockAll) {
        return true;
    } else {
        FOREACH (it, unk90) {
            HamProfile *profile = *it;
            MILO_ASSERT(profile, 0x6d7);
            if (profile->IsDifficultyUnlockedForProfile(s1, s2)) {
                return true;
            }
        }
    }
    return false;
}

bool ProfileMgr::IsContentUnlocked(Symbol s) const {
    if (MetaPanel::sUnlockAll) {
        return true;
    } else {
        FOREACH (it, unk90) {
            HamProfile *profile = *it;
            MILO_ASSERT(profile, 0x680);
            if (profile->IsContentUnlockedForProfile(s)) {
                return true;
            }
        }
    }
    return false;
}

void ProfileMgr::UpdateUsingFitnessState() {
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 0x729);
        bool check = false;
        HamProfile *pProfile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
        if (pProfile) {
            if (pProfile->HasValidSaveData()) {
                check = pProfile->InFitnessMode();
            }
        }
        pPlayer->SetUsingFitness(check);
    }
}

void ProfileMgr::UploadDeferredFitnessGoal() {
    if (unka9) {
        unka9 = false;
        FOREACH (it, unk90) {
            HamProfile *profile = *it;
            MILO_ASSERT(profile, 0x74a);
            TheFitnessGoalMgr->UpdateFitnessGoal(profile);
        }
    }
}

void ProfileMgr::TriggerSignoutEvent() {
    static Symbol sign_out("sign_out");
    static Message init("init", 0);
    init[0] = 0;
    TheUIEventMgr->TriggerEvent(sign_out, init);
    unk90.front()->SetSaveState(kMetaProfileUnloaded);
}

HamProfile *ProfileMgr::GetActiveProfile(bool b) const {
    ShellInput *pShellInput = TheHamUI.GetShellInput();
    if (CriticalProfile()) {
        MILO_ASSERT(pShellInput, 0x565);
        SkeletonChooser *pSkeletonChooser = pShellInput->GetSkeletonChooser();
        MILO_ASSERT(pSkeletonChooser, 0x568);
        int index = pSkeletonChooser->Unk3C();
        HamPlayerData *pActivePlayer = TheGameData->Player(index);
        MILO_ASSERT(pActivePlayer, 0x56c);
        HamProfile *pProfileFromPad =
            TheProfileMgr.GetProfileFromPad(pActivePlayer->PadNum());
        if (!pProfileFromPad || !pProfileFromPad->HasValidSaveData()) {
            HamPlayerData *pOtherPlayer = TheGameData->Player(index == 0);
            MILO_ASSERT(pOtherPlayer, 0x579);
            HamProfile *pOtherPlayerFromPad =
                TheProfileMgr.GetProfileFromPad(pOtherPlayer->PadNum());
            if (!b || pOtherPlayerFromPad || !pOtherPlayerFromPad->HasValidSaveData()) {
                return pOtherPlayerFromPad;
            }
        }
    }
    return nullptr;
}
