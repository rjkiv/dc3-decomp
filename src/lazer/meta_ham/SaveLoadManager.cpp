#include "meta_ham/SaveLoadManager.h"
#include "meta/FixedSizeSaveable.h"
#include "meta/FixedSizeSaveableStream.h"
#include "meta/MemcardMgr.h"
#include "meta/SongMgr.h"
#include "meta_ham/HamMemcardAction.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/UIEventMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Memcard.h"
#include "os/PlatformMgr.h"
#include "ui/UIPanel.h"
#include "utl/BufStream.h"
#include "utl/CacheMgr.h"
#include "utl/Locale.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

namespace {
    const char *kStrGlobalCacheName = "global";
}

SaveLoadManager *TheSaveLoadMgr;

SaveLoadManager::SaveLoadManager()
    : unk2c(0), unk2d(1), mState(), mStateAtSelectStart(), unk3c(-1), unk40(0), unk4c(0),
      unk50(0), mCacheID(0), mCache(0), mData(0), mSongCacheWriteDisabled(0), mWaiting(0),
      unk64(0), unk68(), mNeedsSave(0), mNeedsLoad(0), mLastChosenDeviceID(0),
      mDeviceIDState(0), mAction(0) {
    SetName("saveload_mgr", ObjectDir::Main());
    ThePlatformMgr.AddSink(this, SigninChangedMsg::Type());
}

SaveLoadManager::~SaveLoadManager() {
    ThePlatformMgr.RemoveSink(this, SigninChangedMsg::Type());
    TheUIEventMgr->RemoveSink(this);
    RELEASE(mAction);
}

bool SaveLoadManager::GetDialogFocusOption() { return mState == 0x5c; }

BEGIN_HANDLERS(SaveLoadManager)
    HANDLE_ACTION(autosave, AutoSave())
    HANDLE_ACTION(autoload, AutoLoad())
    HANDLE_ACTION(manual_save, ManualSave(_msg->Obj<HamProfile>(2)))
    HANDLE_EXPR(is_autosave_enabled, IsAutosaveEnabled(_msg->Obj<HamProfile>(2)))
    HANDLE_ACTION(enable_autosave, EnableAutosave(_msg->Obj<HamProfile>(2)))
    HANDLE_ACTION(disable_autosave, DisableAutosave(_msg->Obj<HamProfile>(2)))
    HANDLE_ACTION(handle_eventresponse_start, HandleEventResponseStart(_msg->Int(2)))
    HANDLE_ACTION(
        handle_eventresponse, HandleEventResponse(_msg->Obj<HamProfile>(2), _msg->Int(3))
    )
    HANDLE_EXPR(get_dialog_msg, GetDialogMsg())
    HANDLE_EXPR(get_dialog_opt1, GetDialogOpt1())
    HANDLE_EXPR(get_dialog_opt2, GetDialogOpt2())
    HANDLE_EXPR(get_dialog_focus_option, GetDialogFocusOption())
    HANDLE_EXPR(is_initial_load_done, IsInitialLoadDone())
    HANDLE_EXPR(is_idle, IsIdle())
    HANDLE_ACTION(activate, Activate())
    HANDLE_ACTION(printout_savesize_info, PrintoutSaveSizeInfo())
    HANDLE_MESSAGE(DeviceChosenMsg)
    HANDLE_MESSAGE(NoDeviceChosenMsg)
    HANDLE_MESSAGE(MCResultMsg)
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_MESSAGE(EventDialogDismissMsg)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void SaveLoadManager::AutoSave() {
    if (IsReasonToAutosave()) {
        mNeedsSave = true;
    }
}

void SaveLoadManager::AutoLoad() {
    if (IsReasonToAutoload()) {
        mNeedsLoad = true;
    }
}

void SaveLoadManager::HandleEventResponseStart(int) { mStateAtSelectStart = mState; }

__forceinline bool SaveLoadManager::IsIdle() const {
    return mState == 0 && (!unk2c || (!mNeedsSave && !mNeedsLoad));
}

void SaveLoadManager::PrintoutSaveSizeInfo() {
    FixedSizeSaveable::EnablePrintouts(true);
    MILO_LOG("SAVESIZE\n");
    int profileSize = HamProfile::SaveSize(0x5C);
    int symTableSize = FixedSizeSaveableStream::GetSymbolTableSize(0x5C);
    MILO_LOG("Symbol Table Size = %i\n", symTableSize);
    MILO_LOG("SAVESIZE TOTAL = %i \n", symTableSize + profileSize);
}

bool SaveLoadManager::IsReasonToAutosave() {
    HamProfile *p = GetAutosavableProfile();
    return p || TheProfileMgr.GlobalOptionsNeedsSave() || SongCacheNeedsWrite();
}

bool SaveLoadManager::IsReasonToAutoload() {
    HamProfile *p = GetNewSigninProfile();
    return p || unk2d;
}

void SaveLoadManager::EnableAutosave(HamProfile *p) {
    if (!p) {
        MILO_NOTIFY("Tried to enable autosave without a valid profile.");
    } else {
        ManualSave(p);
    }
}

void SaveLoadManager::ManualSave(HamProfile *pProfile) {
    State cur = mState;
    if (cur != 0) {
        MILO_NOTIFY(
            "Attempted to perform a manual save, but saveloadmgr is not idle (state = %d).",
            cur
        );
    } else {
        MILO_ASSERT(pProfile, 0x364);
        unk40 = pProfile;
        unk3c = pProfile->GetPadNum();
        TheMemcardMgr.AddSink(this);
        SetState((State)0x56);
    }
}

void SaveLoadManager::Start() {
    unk3c = -1;
    TheMemcardMgr.AddSink(this);
    SetState((State)1);
    if (mMode == 0) {
        UpdateStatus((SaveLoadMgrStatus)3);
    }
}

void SaveLoadManager::Finish() {
    if (mMode == 0) {
        UpdateStatus((SaveLoadMgrStatus)4);
    }
    TheMemcardMgr.RemoveSink(this);
    SetState(kS_Finish);
}

void SaveLoadManager::UpdateStatus(SaveLoadMgrStatus status) {
    static SaveLoadMgrStatusUpdateMsg msg(-1);
    msg[0] = status;
    Export(msg, true);
}

bool SaveLoadManager::SongCacheNeedsWrite() {
    return TheSongMgr.SongCacheNeedsWrite() && !mSongCacheWriteDisabled;
}

void SaveLoadManager::DisableAutosave(HamProfile *pProfile) {
    if (!pProfile) {
        MILO_NOTIFY("Tried to disable autosave without a valid profile.");
    } else if (!IsIdle()) {
        MILO_NOTIFY("Tried to disable autosave while saveloadmgr is not idle.");
    } else {
        pProfile->SetSaveState(kMetaProfileError); // error?
    }
}

bool SaveLoadManager::IsSafePlaceToSave() const {
    if (!TheUIEventMgr->HasActiveDialogEvent()
        && !TheUIEventMgr->HasActiveTransitionEvent()) {
        return true;
    } else
        return false;
}

bool SaveLoadManager::IsSafePlaceToLoad() const {
    if (TheUIEventMgr->HasActiveDialogEvent()
        || TheUIEventMgr->HasActiveTransitionEvent()) {
        return false;
    } else {
        bool ret = true;
        UIPanel *panel = TheHamUI.FocusPanel();
        if (panel) {
            static Symbol allow_load("allow_load");
            const DataNode *n = panel->Property(allow_load, false);
            if (n) {
                ret = n->Int();
            }
        }
        return ret;
    }
}

void SaveLoadManager::Activate() {
    if (!unk2c) {
        unk2c = true;
        mNeedsLoad = true;
        TheUIEventMgr->AddSink(this, EventDialogDismissMsg::Type());
    }
}

bool SaveLoadManager::IsAutosaveEnabled(HamProfile *p) {
    if (!p) {
        MILO_NOTIFY("Tried to get autosave enabled status without a valid profile.");
        return false;
    } else {
        return p->IsAutosaveEnabled();
    }
}

Symbol SaveLoadManager::GetDialogOpt1() {
    static Symbol mc_button_create_data("mc_button_create_data");
    static Symbol mc_button_choose_device("mc_button_choose_device");
    static Symbol mc_button_continue("mc_button_continue");
    static Symbol mc_button_overwrite("mc_button_overwrite");
    static Symbol song_info_cache_button_create("song_info_cache_button_create");
    static Symbol song_info_cache_button_corrupt_overwrite(
        "song_info_cache_button_corrupt_overwrite"
    );
    static Symbol global_options_button_create("global_options_button_create");
    static Symbol global_options_button_corrupt_overwrite(
        "global_options_button_corrupt_overwrite"
    );
    static Symbol mc_button_delete_saves("mc_button_delete_saves");
    static Symbol mc_button_yes("mc_button_yes");
    Symbol out = gNullStr;
    switch (mState) {
    case kS_AutoloadNoSaveFound_Msg:
        out = mc_button_create_data;
        break;
    case kS_AutoloadCorrupt:
    case kS_AutoloadNotOwner:
    case kS_AutoloadObsolete:
    case kS_AutoloadFuture:
    case kS_SaveConfirmOverwrite:
        out = mc_button_overwrite;
        break;
    case kS_SongCacheCreateNotFound_Msg:
    case kS_SongCacheCreateMissing_Msg:
        out = song_info_cache_button_create;
        break;
    case kS_SongCacheCreateCorrupt:
        out = song_info_cache_button_corrupt_overwrite;
        break;
    case kS_GlobalCreateCorrupt:
        out = global_options_button_corrupt_overwrite;
        break;
    case kS_GlobalCreateNotFound_Msg:
    case kS_GlobalCreateMissing_Msg:
    case kS_GlobalOptionsMissing_Msg:
        out = global_options_button_create;
        break;
    case kS_SaveNotEnoughSpacePS3:
        out = mc_button_delete_saves;
        break;
    case kS_AutoloadMultipleSavesFound:
    case kS_AutoloadDeviceMissing:
    case kS_SaveDeviceInvalid:
    case kS_ManualSaveNoDevice:
    case kS_ManualLoadNoDevice:
        out = mc_button_choose_device;
        break;
    case kS_ManualLoadConfirmUnsaved:
        out = mc_button_continue;
        break;
    case kS_ManualLoadConfirm:
        out = mc_button_yes;
        break;
    default:
        break;
    }
    return out;
}

Symbol SaveLoadManager::GetDialogOpt2() {
    static Symbol mc_button_cancel("mc_button_cancel");
    static Symbol mc_button_continue_no_save("mc_button_continue_no_save");
    static Symbol song_info_cache_button_cancel("song_info_cache_button_cancel");
    static Symbol global_options_button_cancel("global_options_button_cancel");
    static Symbol mc_button_retry("mc_button_retry");
    static Symbol mc_button_disable_autosave("mc_button_disable_autosave");
    static Symbol mc_button_no("mc_button_no");
    Symbol out = gNullStr;
    switch (mState) {
    case kS_AutoloadCorrupt:
    case kS_AutoloadNotOwner:
    case kS_AutoloadObsolete:
    case kS_AutoloadFuture:
        out = mc_button_continue_no_save;
        break;
    case kS_SongCacheCreateNotFound_Msg:
    case kS_SongCacheCreateMissing_Msg:
    case kS_SongCacheCreateCorrupt:
        out = song_info_cache_button_cancel;
        break;
    case kS_GlobalCreateNotFound_Msg:
    case kS_GlobalCreateMissing_Msg:
    case kS_GlobalCreateCorrupt:
    case kS_GlobalOptionsMissing_Msg:
        out = global_options_button_cancel;
        break;
    case kS_SaveNotEnoughSpacePS3:
        out = mc_button_retry;
        break;
    case kS_SaveDeviceInvalid:
        out = mc_button_disable_autosave;
        break;
    case kS_ManualLoadConfirm:
        out = mc_button_no;
        break;
    case kS_AutoloadNoSaveFound_Msg:
    case kS_AutoloadMultipleSavesFound:
    case kS_AutoloadDeviceMissing:
    case kS_SaveConfirmOverwrite:
    case kS_ManualSaveNoDevice:
    case kS_ManualLoadConfirmUnsaved:
    case kS_ManualLoadNoDevice:
        out = mc_button_cancel;
        break;
    default:
        break;
    }
    return out;
}

void SaveLoadManager::Init() {
    MILO_ASSERT(!TheSaveLoadMgr, 0x47);
    TheSaveLoadMgr = new SaveLoadManager();
}

HamProfile *SaveLoadManager::GetAutosavableProfile() {
    std::vector<HamProfile *> shouldAutosaves = TheProfileMgr.GetShouldAutosave();
    if (!shouldAutosaves.empty()) {
        HamProfile *pProfile = shouldAutosaves[0];
        MILO_ASSERT(pProfile, 0x401);
        return pProfile;
    } else {
        return nullptr;
    }
}

HamProfile *SaveLoadManager::GetNewSigninProfile() {
    std::vector<HamProfile *> signIns = TheProfileMgr.GetNewlySignedIn();
    if (!signIns.empty()) {
        HamProfile *pProfile = signIns[0];
        MILO_ASSERT(pProfile, 0x3F2);
        return pProfile;
    } else {
        return nullptr;
    }
}

DataNode SaveLoadManager::OnMsg(const DeviceChosenMsg &msg) {
    MILO_ASSERT(mWaiting, 0x887);
    mWaiting = false;
    switch (mState) {
    case kS_ManualSaveChooseDevice:
        mLastChosenDeviceID = msg.Device();
        SetState(kS_SaveLookForFile);
        break;
    case 8:
    case 9:
    case 0xA:
    case 0xD:
        mLastChosenDeviceID = msg.Device();
        SetState(kS_AutoloadStartLoad);
        break;
    case kS_SaveChooseDeviceInvalid:
        SetState(kS_SaveNoOverwrite);
        break;
    case kS_ManualLoadChooseDevice:
        SetState(kS_ManualLoadStartLoad);
        break;
    case kS_Abort:
    case kS_Done:
    case kS_Finish:
        break;
    default:
        State state = mState;
        SaveLoadMode mode = mMode;
        MILO_FAIL("Unhandled DeviceChosenMsg in state %d and mode %d", state, mode);
        break;
    }
    return 0;
}

DataNode SaveLoadManager::OnMsg(const EventDialogDismissMsg &msg) {
    static Symbol saveload_dialog_event("saveload_dialog_event");
    Symbol s2 = msg->ForceSym(2);
    Symbol s3 = msg->ForceSym(3);
    if (s3 != gNullStr && s2 == saveload_dialog_event && s3 != saveload_dialog_event) {
        SetState(kS_Abort);
    }
    return DATA_UNHANDLED;
}

DataNode SaveLoadManager::GetDialogMsg() {
    String profileName = gNullStr;
    int playerNum = -1;
    if (unk40) {
        profileName = unk40->GetName();
        playerNum = unk40->GetPadNum() + 1;
    }
    switch (mState) {
    case kS_AutoloadNoSaveFound_Msg: {
        static Symbol mc_auto_load_no_save_found_fmt("mc_auto_load_no_save_found_fmt");
        return DataArrayPtr(mc_auto_load_no_save_found_fmt, DataArrayPtr(), profileName);
    }
    case kS_AutoloadMultipleSavesFound: {
        static Symbol mc_auto_load_multiple_saves_found_fmt(
            "mc_auto_load_multiple_saves_found_fmt"
        );
        return DataArrayPtr(
            mc_auto_load_multiple_saves_found_fmt, DataArrayPtr(), profileName
        );
    }
    case kS_AutoloadDeviceMissing: {
        static Symbol mc_load_device_missing_fmt("mc_load_device_missing_fmt");
        return DataArrayPtr(mc_load_device_missing_fmt, DataArrayPtr(), profileName);
    }
    case kS_AutoloadCorrupt: {
        static Symbol mc_auto_load_corrupt("mc_auto_load_corrupt");
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0xAD6);
        return DataArrayPtr(
            mc_auto_load_corrupt,
            DataArrayPtr(),
            ThePlatformMgr.GetName(pProfile->GetPadNum())
        );
    }
    case kS_AutoloadNotOwner: {
        static Symbol mc_auto_load_not_owner("mc_auto_load_not_owner");
        return DataArrayPtr(mc_auto_load_not_owner, DataArrayPtr());
    }
    case kS_AutoloadObsolete: {
        if (playerNum != -1) {
            static Symbol mc_auto_load_obsolete_version_fmt(
                "mc_auto_load_obsolete_version_fmt"
            );
            return DataArrayPtr(
                mc_auto_load_obsolete_version_fmt, DataArrayPtr(), profileName
            );
        } else {
            static Symbol mc_auto_load_obsolete_version("mc_auto_load_obsolete_version");
            return DataArrayPtr(mc_auto_load_obsolete_version, DataArrayPtr());
        }
    }
    case kS_AutoloadFuture: {
        if (playerNum != -1) {
            static Symbol mc_auto_load_newer_version_fmt("mc_auto_load_newer_version_fmt");
            return DataArrayPtr(
                mc_auto_load_newer_version_fmt, DataArrayPtr(), profileName
            );
        } else {
            static Symbol mc_auto_load_newer_version("mc_auto_load_newer_version");
            return DataArrayPtr(mc_auto_load_newer_version, DataArrayPtr());
        }
    }
    case kS_SongCacheCreateNotFound_Msg: {
        static Symbol song_info_cache_create("song_info_cache_create");
        return DataArrayPtr(song_info_cache_create, DataArrayPtr());
    }
    case kS_SongCacheCreateMissing_Msg: {
        static Symbol song_info_cache_missing("song_info_cache_missing");
        return DataArrayPtr(song_info_cache_missing, DataArrayPtr());
    }
    case kS_SongCacheCreateCorrupt: {
        static Symbol song_info_cache_corrupt("song_info_cache_corrupt");
        return DataArrayPtr(song_info_cache_corrupt, DataArrayPtr());
    }
    case kS_GlobalCreateNotFound_Msg: {
        static Symbol global_options_create("global_options_create");
        return DataArrayPtr(global_options_create, DataArrayPtr());
    }
    case kS_GlobalCreateMissing_Msg:
    case kS_GlobalOptionsMissing_Msg: {
        static Symbol global_options_missing("global_options_missing");
        return DataArrayPtr(global_options_missing, DataArrayPtr());
    }
    case kS_GlobalCreateCorrupt: {
        static Symbol global_options_corrupt("global_options_corrupt");
        return DataArrayPtr(global_options_corrupt, DataArrayPtr());
    }
    case kS_SaveLoadError: {
        static Symbol mc_autosave_disabled("mc_autosave_disabled");
        return DataArrayPtr(mc_autosave_disabled, DataArrayPtr());
    }
    case kS_SaveConfirmOverwrite: {
        static Symbol mc_save_confirm_overwrite("mc_save_confirm_overwrite");
        return DataArrayPtr(mc_save_confirm_overwrite, DataArrayPtr());
    }
    case kS_SaveNotEnoughSpace: {
        static Symbol mc_save_not_enough_space("mc_save_not_enough_space");
        return DataArrayPtr(mc_save_not_enough_space, DataArrayPtr());
    }
    case kS_SaveNotEnoughSpacePS3: {
        static Symbol mc_save_not_enough_space("mc_save_not_enough_space");
        return DataArrayPtr(
            mc_save_not_enough_space, DataArrayPtr(), -TheMemcardMgr.GetSizeNeeded()
        );
    }
    case kS_SaveDeviceInvalid: {
        static Symbol mc_save_device_missing_fmt("mc_save_device_missing_fmt");
        return DataArrayPtr(mc_save_device_missing_fmt, DataArrayPtr(), profileName);
    }
    case kS_SaveFailed: {
        static Symbol mc_save_failed("mc_save_failed");
        return DataArrayPtr(mc_save_failed, DataArrayPtr());
    }
    case kS_SaveDisabledByCheat: {
        static Symbol mc_save_disabled_by_cheat("mc_save_disabled_by_cheat");
        return DataArrayPtr(mc_save_disabled_by_cheat, DataArrayPtr());
    }
    case kS_LoadFailed: {
        static Symbol mc_load_failed("mc_load_failed");
        return DataArrayPtr(mc_load_failed, DataArrayPtr());
    }
    case kS_ManualSaveNoDevice: {
        static Symbol mc_manual_save_no_selection("mc_manual_save_no_selection");
        return DataArrayPtr(mc_manual_save_no_selection, DataArrayPtr());
    }
    case kS_ManualLoadConfirmUnsaved: {
        if (playerNum != -1) {
            static Symbol mc_manual_load_confirm_unsaved_fmt(
                "mc_manual_load_confirm_unsaved_fmt"
            );
            return DataArrayPtr(
                mc_manual_load_confirm_unsaved_fmt, DataArrayPtr(), profileName
            );
        } else {
            static Symbol mc_manual_load_confirm_unsaved("mc_manual_load_confirm_unsaved");
            return DataArrayPtr(mc_manual_load_confirm_unsaved, DataArrayPtr());
        }
    }
    case kS_ManualLoadConfirm: {
        static Symbol mc_manual_load_confirm("mc_manual_load_confirm");
        return DataArrayPtr(mc_manual_load_confirm, DataArrayPtr());
    }
    case kS_ManualLoadNoDevice: {
        static Symbol mc_manual_load_no_selection("mc_manual_load_no_selection");
        return DataArrayPtr(mc_manual_load_no_selection, DataArrayPtr());
    }
    case kS_ManualLoadMissing: {
        static Symbol mc_manual_load_storage_missing("mc_manual_load_storage_missing");
        return DataArrayPtr(mc_manual_load_storage_missing, DataArrayPtr());
    }
    case kS_ManualLoadNoFile: {
        static Symbol mc_manual_load_no_file("mc_manual_load_no_file");
        return DataArrayPtr(mc_manual_load_no_file, DataArrayPtr());
    }
    case kS_ManualLoadCorrupt: {
        static Symbol mc_manual_load_corrupt("mc_manual_load_corrupt");
        return DataArrayPtr(mc_manual_load_corrupt, DataArrayPtr());
    }
    case kS_ManualLoadNotOwner: {
        static Symbol mc_manual_load_not_owner("mc_manual_load_not_owner");
        return DataArrayPtr(mc_manual_load_not_owner, DataArrayPtr());
    }
    default: {
        MILO_ASSERT(false, 0xB73);
        return 0;
    }
    }
}

void SaveLoadManager::SetState(State newState) {
    if (mState == newState)
        return;

    static Symbol saveload_dialog_event("saveload_dialog_event");

    // Cleanup logic - structure must match assembly branch pattern
    bool wasIdle = false;

    // Check for states that need mData or mAction cleanup
    if (mState > kS_GlobalOptionsWrite) {
        // curState > 0x3E: check for mAction release states
        if (mState >= kS_SaveOverwrite) {
            if (mState <= kS_SaveNoOverwrite) {
                // 0x46-0x47: release mAction unless going to Abort
                if (newState != kS_Abort) {
                    RELEASE(mAction);
                }
            } else if (mState == kS_ManualLoadStartLoad) {
                // 0x60: release mAction unless going to Abort
                if (newState != kS_Abort) {
                    RELEASE(mAction);
                }
            } else if (mState == kS_Abort) {
                // 0x65: release mAction
                RELEASE(mAction);
            } else if (mState == kS_Finish) {
                // 0x67: free mData
                if (mData) {
                    MemFree(mData, "SaveLoadManager.cpp", 0x433);
                    mData = nullptr;
                }
            }
        }
    } else if (mState == kS_GlobalOptionsWrite ||
               mState == kS_SongCacheWrite ||
               mState == kS_SongCacheDone ||
               (mState > kS_GlobalDoneRead && mState <= kS_GlobalDoneWrite)) {
        // States 0x3E, 0x1F, 0x21, 0x32-0x33: free mData unless going to Finish
        if (newState != kS_Finish) {
            if (mData) {
                MemFree(mData, "SaveLoadManager.cpp", 0x424);
                mData = nullptr;
            }
        }
    } else if (mState == kS_Idle) {
        // State 0: set wasIdle flag
        wasIdle = true;
    } else if (mState == kS_AutoloadStartLoad) {
        // State 0xB: release mAction unless going to Abort
        if (newState != kS_Abort) {
            RELEASE(mAction);
        }
    }

    mState = newState;

    if (wasIdle) {
        UpdateStatus((SaveLoadMgrStatus)0);
    }

    // Handle state based on new state value
    if (mState > kS_Done)
        return;

    switch (mState) {
    case kS_Idle:
        UpdateStatus((SaveLoadMgrStatus)5);
        break;
    case kS_Start:
        mDeviceIDState = 0;
        break;
    case kS_AutoloadInit:
        if (unk2d) {
            SetState(kS_SongCacheInit);
        } else {
            SetState(kS_AutoloadSelectProfile);
        }
        break;
    case kS_AutoloadSelectProfile:
        unk40 = GetNewSigninProfile();
        if (unk40) {
            SetState(kS_AutoloadSearchDevice);
        } else {
            SetState(kS_AutoloadDone);
        }
        break;
    case kS_AutoloadSearchDevice: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x48B);
        mWaiting = true;
        TheMemcardMgr.OnSearchForDevice(pProfile);
        break;
    }
    case kS_AutoloadDeviceFound:
        if (mDeviceIDState == 2) {
            SetState(kS_AutoloadSetDevice);
        } else {
            SetState(kS_AutoloadNoSaveFound_Msg);
        }
        break;
    case kS_AutoloadNoSaveFound_Msg:
        // Dialog state - wait for user response
        break;
    case kS_AutoloadMultipleSavesFound:
        // Dialog state - wait for user response
        break;
    case kS_AutoloadSetDevice:
        MILO_ASSERT(mDeviceIDState == 2, 0x4AB);
        TheMemcardMgr.SetDevice(mLastChosenDeviceID);
        SetState(kS_AutoloadStartLoad);
        break;
    case kS_AutoloadSelectDevice: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x4B6);
        mWaiting = true;
        TheMemcardMgr.SelectDevice(pProfile, this, unk3c, false);
        break;
    }
    case kS_AutoloadSelectDevice2: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x4C7);
        mWaiting = true;
        TheMemcardMgr.SelectDevice(pProfile, this, unk3c, false);
        break;
    }
    case kS_AutoloadStartLoad: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x4D6);
        mWaiting = true;
        RELEASE(mAction);
        mAction = new LoadMemcardAction(pProfile);
        pProfile->PreLoad();
        TheMemcardMgr.OnLoadGame(pProfile, mAction);
        break;
    }
    case kS_AutoloadDeviceMissing:
        // Dialog state
        break;
    case kS_AutoloadSelectDevice3: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x4B6);
        mWaiting = true;
        TheMemcardMgr.SelectDevice(pProfile, this, unk3c, true);
        break;
    }
    case kS_AutoloadCorrupt:
    case kS_AutoloadNotOwner:
    case kS_AutoloadObsolete:
    case kS_AutoloadFuture:
        // Dialog states
        break;
    case kS_AutoloadDone:
        unk2d = false;
        if (TheProfileMgr.GlobalOptionsNeedsSave()) {
            SetState(kS_SongCacheInit);
        } else {
            TheProfileMgr.HandleProfileLoadComplete();
            SetState(kS_Done);
        }
        break;
    case kS_SongCacheInit: {
        unk44 = TheSongMgr.GetCachedSongInfoName();
        if (mCacheID) {
            TheCacheMgr->RemoveCacheID(mCacheID);
            RELEASE(mCacheID);
        }
        if (!TheCacheMgr->SearchAsync(unk44.c_str(), &mCacheID)) {
            MILO_FAIL("TheCacheMgr->SearchAsync() failed. CacheResult = %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    }
    case kS_SongCacheSearch:
        // Waiting for search result
        break;
    case kS_SongCacheSearchResult:
        SetState(kS_SongCacheMount);
        break;
    case kS_SongCacheCreate: {
        if (mCacheID) {
            TheCacheMgr->RemoveCacheID(mCacheID);
            RELEASE(mCacheID);
        }
        static Symbol song_info_cache_name("song_info_cache_name");
        const char *cacheName = Localize(song_info_cache_name, nullptr, TheLocale);
        if (!TheCacheMgr->ShowUserSelectUIAsync(nullptr, 0x25800, unk44.c_str(), cacheName, &mCacheID)) {
            int result = TheCacheMgr->GetLastResult();
            if (result != 0) {
                SetState(kS_SongCacheMountStart);
            }
        }
        break;
    }
    case kS_SongCacheCreateNotFound_Msg:
    case kS_SongCacheCreateMissing_Msg:
        // Dialog states
        break;
    case kS_SongCacheMount: {
        if (!TheCacheMgr->MountAsync(mCacheID, &mCache, nullptr)) {
            MILO_FAIL("TheCacheMgr->MountAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    }
    case kS_SongCacheMountStart:
        UpdateStatus((SaveLoadMgrStatus)1);
        // Fall through to mount logic handled elsewhere
        {
            if (!TheCacheMgr->MountAsync(mCacheID, &mCache, nullptr)) {
                MILO_FAIL("TheCacheMgr->MountAsync failed with CacheResult %d",
                          TheCacheMgr->GetLastResult());
            }
        }
        break;
    case kS_SongCacheRead:
        UpdateStatus((SaveLoadMgrStatus)1);
        if (!TheCacheMgr->DeleteAsync(mCacheID)) {
            MILO_FAIL("TheCacheMgr->DeleteAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    case kS_SongCacheCreateCorrupt:
        // Dialog state
        break;
    case kS_SongCacheGetSize:
        if (!mCache->GetFileSizeAsync(unk44.c_str(), (unsigned int *)&unk4c, nullptr)) {
            MILO_FAIL("mCache->GetFileSizeAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    case kS_SongCacheAllocRead:
        mData = _MemAllocTemp(unk4c, "SaveLoadManager.cpp", 0x578, "SaveLoadManager", 0);
        if (!mCache->ReadAsync(unk44.c_str(), mData, unk4c, nullptr)) {
            MILO_FAIL("mCache->ReadAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    case kS_SongCacheWrite: {
        int size = TheSongMgr.GetCachedSongInfoSize();
        mData = _MemAllocTemp(size, "SaveLoadManager.cpp", 0x595, "SaveLoadManager", 0);
        BufStream bs(mData, size, true);
        if (TheSongMgr.SaveCachedSongInfo(bs)) {
            if (!mCache->WriteAsync(unk44.c_str(), mData, size, nullptr)) {
                MILO_FAIL("mCache->WriteAsync failed with CacheResult %d",
                          TheCacheMgr->GetLastResult());
            }
        }
        break;
    }
    case kS_SongCacheUnmount:
        if (!TheCacheMgr->UnmountAsync(&mCache, nullptr)) {
            MILO_FAIL("TheCacheMgr->UnmountAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    case kS_SongCacheDone:
        mDeviceIDState = 1;
        mLastChosenDeviceID = 0;
        mSongCacheWriteDisabled = true;
        if (mCache) {
            SetState(kS_SongCacheLookup);
        } else {
            SetState((State)(kS_SongCacheLookup - 4));
        }
        break;
    case kS_SongCacheFailed:
        mDeviceIDState = 0;
        mLastChosenDeviceID = 0;
        mSongCacheWriteDisabled = true;
        if (mCache) {
            SetState(kS_SongCacheLookup);
        } else {
            SetState((State)(kS_SongCacheLookup - 4));
        }
        break;
    case kS_SongCacheLookup:
        mCacheID = nullptr;
        SetState(kS_GlobalOptionsCreate);
        break;
    case kS_GlobalOptionsInit: {
        if (!mCacheID) {
            Symbol globalCacheName(kStrGlobalCacheName);
            mCacheID = TheCacheMgr->GetCacheID(globalCacheName);
        }
        if (!mCacheID) {
            SetState(kS_GlobalCacheLookup);
        } else {
            SetState(kS_GlobalDoneRead);
        }
        break;
    }
    case kS_GlobalOptionsSearch: {
        if (mCacheID) {
            TheCacheMgr->RemoveCacheID(mCacheID);
            RELEASE(mCacheID);
        }
        if (!TheCacheMgr->SearchAsync(kStrGlobalCacheName, &mCacheID)) {
            MILO_FAIL("TheCacheMgr->SearchAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    }
    case kS_GlobalOptionsSearchResult:
        if (mDeviceIDState == 0) {
            mDeviceIDState = 0;
            SetState(kS_GlobalCreateMissing_Msg);
        } else {
            SetState(kS_GlobalMount);
        }
        break;
    case kS_GlobalOptionsCreate: {
        if (mCacheID) {
            TheCacheMgr->RemoveCacheID(mCacheID);
            RELEASE(mCacheID);
        }
        static Symbol global_options_cache_name("global_options_cache_name");
        int saveSize = TheProfileMgr.GlobalOptionsSaveSize();
        const char *cacheName = Localize(global_options_cache_name, nullptr, TheLocale);
        if (!TheCacheMgr->ShowUserSelectUIAsync(nullptr, saveSize, kStrGlobalCacheName, cacheName, &mCacheID)) {
            int result = TheCacheMgr->GetLastResult();
            if (result != 0) {
                SetState(kS_GlobalCreate2);
            }
        }
        break;
    }
    case kS_GlobalOptionsLookup: {
        if (mCacheID) {
            TheCacheMgr->RemoveCacheID(mCacheID);
            RELEASE(mCacheID);
        }
        Symbol globalCacheName(kStrGlobalCacheName);
        mCacheID = TheCacheMgr->GetCacheID(globalCacheName);
        if (!mCacheID) {
            SetState(kS_GlobalOptionsMissing_Msg);
        } else {
            SetState(kS_GlobalMount2);
        }
        break;
    }
    case kS_GlobalCreateNotFound_Msg:
    case kS_GlobalCreateMissing_Msg:
        // Dialog states
        break;
    case kS_GlobalMount:
        // Wait for mount
        break;
    case kS_GlobalMountStart:
        // Start mount
        break;
    case kS_GlobalCreate2:
        MILO_ASSERT(mDeviceIDState == 2, 0x627);
        // fall through to mount
        {
            if (mCacheID) {
                TheCacheMgr->RemoveCacheID(mCacheID);
                RELEASE(mCacheID);
            }
            static Symbol global_options_cache_name("global_options_cache_name");
            const char *cacheName = Localize(global_options_cache_name, nullptr, TheLocale);
            TheCacheMgr->CreateCacheIDFromDeviceID(mLastChosenDeviceID, kStrGlobalCacheName,
                                                   cacheName, &mCacheID);
        }
        break;
    case kS_GlobalMount2:
        break;
    case kS_GlobalCreateCorrupt:
        // Dialog state
        break;
    case kS_GlobalRead: {
        int saveSize = TheProfileMgr.GlobalOptionsSaveSize();
        mData = _MemAllocTemp(saveSize, "SaveLoadManager.cpp", 0x69B, "SaveLoadManager", 0);
        if (!mCache->ReadAsync(kStrGlobalCacheName, mData, saveSize, nullptr)) {
            MILO_FAIL("TheCacheMgr->ReadAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    }
    case kS_GlobalDoneRead:
        // Handle read completion
        break;
    case kS_GlobalWrite: {
        UpdateStatus((SaveLoadMgrStatus)1);
        int saveSize = TheProfileMgr.GlobalOptionsSaveSize();
        mData = _MemAllocTemp(saveSize, "SaveLoadManager.cpp", 0x6AD, "SaveLoadManager", 0);
        FixedSizeSaveableStream fs(mData, saveSize, true);
        TheProfileMgr.SaveGlobalOptions(fs);
        if (!mCache->WriteAsync(kStrGlobalCacheName, mData, saveSize, nullptr)) {
            MILO_FAIL("mCache->WriteAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    }
    case kS_GlobalDoneWrite:
        // Handle write completion
        break;
    case kS_GlobalUnmount:
        if (!TheCacheMgr->UnmountAsync(&mCache, nullptr)) {
            int result = TheCacheMgr->GetLastResult();
            if (result != kCache_ErrorStorageDeviceMissing) {
                MILO_NOTIFY("UnmountAsync failed with error %d",
                            TheCacheMgr->GetLastResult());
            }
        }
        break;
    case kS_GlobalDone:
        mDeviceIDState = 1;
        mLastChosenDeviceID = 0;
        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
        SetState(kS_GlobalNewSignIns);
        break;
    case kS_GlobalFailed:
        mDeviceIDState = 0;
        mLastChosenDeviceID = 0;
        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
        SetState(kS_GlobalNewSignIns);
        break;
    case kS_GlobalCacheLookup:
        // Look up cache
        break;
    case kS_GlobalNewSignIns: {
        std::vector<HamProfile *> newSignIns = TheProfileMgr.GetNewlySignedIn();
        bool hasMultiple = newSignIns.size() > 1;
        if (hasMultiple) {
            mDeviceIDState = 1;
        }
        SetState(kS_AutoloadSelectProfile);
        break;
    }
    case kS_GlobalOptionsSearchResult2:
        if (mDeviceIDState == 0 || mDeviceIDState == 2) {
            SetState(kS_GlobalOptionsMissing_Msg);
        } else {
            SetState(kS_GlobalOptionsCreate2);
        }
        break;
    case kS_GlobalOptionsMissing_Msg:
        // Dialog state
        break;
    case kS_GlobalOptionsCreate2: {
        if (mCacheID) {
            TheCacheMgr->RemoveCacheID(mCacheID);
            RELEASE(mCacheID);
        }
        static Symbol global_options_cache_name("global_options_cache_name");
        int saveSize = TheProfileMgr.GlobalOptionsSaveSize();
        const char *cacheName = Localize(global_options_cache_name, nullptr, TheLocale);
        if (!TheCacheMgr->ShowUserSelectUIAsync(nullptr, saveSize, kStrGlobalCacheName, cacheName, &mCacheID)) {
            int result = TheCacheMgr->GetLastResult();
            if (result != 0) {
                SetState(kS_GlobalOptionsRead);
            }
        }
        break;
    }
    case kS_GlobalOptionsRead:
        // Read options
        break;
    case kS_GlobalOptionsAllocRead: {
        int saveSize = TheProfileMgr.GlobalOptionsSaveSize();
        mData = _MemAllocTemp(saveSize, "SaveLoadManager.cpp", 0x69B, "SaveLoadManager", 0);
        if (!mCache->ReadAsync(kStrGlobalCacheName, mData, saveSize, nullptr)) {
            MILO_FAIL("TheCacheMgr->ReadAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    }
    case kS_GlobalOptionsWrite: {
        UpdateStatus((SaveLoadMgrStatus)1);
        int saveSize = TheProfileMgr.GlobalOptionsSaveSize();
        mData = _MemAllocTemp(saveSize, "SaveLoadManager.cpp", 0x6AD, "SaveLoadManager", 0);
        FixedSizeSaveableStream fs(mData, saveSize, true);
        TheProfileMgr.SaveGlobalOptions(fs);
        if (!mCache->WriteAsync(kStrGlobalCacheName, mData, saveSize, nullptr)) {
            MILO_FAIL("mCache->WriteAsync failed with CacheResult %d",
                      TheCacheMgr->GetLastResult());
        }
        break;
    }
    case kS_GlobalOptionsUnmount:
        if (!TheCacheMgr->UnmountAsync(&mCache, nullptr)) {
            int result = TheCacheMgr->GetLastResult();
            if (result != kCache_ErrorStorageDeviceMissing) {
                MILO_NOTIFY("UnmountAsync failed with error %d",
                            TheCacheMgr->GetLastResult());
            }
        }
        break;
    case kS_GlobalOptionsFailed:
        mDeviceIDState = 0;
        mLastChosenDeviceID = 0;
        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
        SetState(kS_GlobalOptionsDone);
        break;
    case kS_GlobalOptionsDone:
        mDeviceIDState = 1;
        mLastChosenDeviceID = 0;
        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
        SetState(kS_GlobalNewSignIns);
        break;
    case kS_SaveLoadError: {
        HamProfile *pProfile = unk40;
        mDeviceIDState = 0;
        MILO_ASSERT(pProfile, 0x6FE);
        TheMemcardMgr.SaveLoadProfileComplete(pProfile, 2);
        TheUIEventMgr->TriggerEvent(saveload_dialog_event, nullptr);
        break;
    }
    case kS_SaveLoadError2: {
        int errorType = 1;
        mDeviceIDState = 0;
        if (mState == kS_SaveLoadError2) {
            errorType = -1;
        }
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x713);
        TheMemcardMgr.SaveLoadProfileComplete(pProfile, errorType);
        if (mMode >= kAutoSave) {
            if (mMode == kAutoSave) {
                SetState(kS_SaveCheckProfile);
            }
        } else {
            SetState(kS_AutoloadSelectProfile);
        }
        break;
    }
    case kS_SaveLoadCheckForFile: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x72D);
        mWaiting = true;
        TheMemcardMgr.OnCheckForSaveContainer(pProfile);
        break;
    }
    case kS_SaveLookForFile: {
        UpdateStatus((SaveLoadMgrStatus)1);
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x739);
        mWaiting = true;
        RELEASE(mAction);
        mAction = new SaveMemcardAction(pProfile);
        TheMemcardMgr.OnSaveGame(pProfile, mAction, 1);
        break;
    }
    case kS_SaveOverwrite: {
        UpdateStatus((SaveLoadMgrStatus)1);
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x747);
        mWaiting = true;
        RELEASE(mAction);
        mAction = new SaveMemcardAction(pProfile);
        TheMemcardMgr.OnSaveGame(pProfile, mAction, 0);
        break;
    }
    case kS_SaveNoOverwrite:
        // Save without overwrite
        break;
    case kS_SaveConfirmOverwrite:
    case kS_SaveNotEnoughSpace:
    case kS_SaveNotEnoughSpacePS3:
        // Dialog states
        break;
    case kS_SaveDeleteSaves: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x76D);
        mWaiting = true;
        TheMemcardMgr.OnDeleteSaves(pProfile);
        break;
    }
    case kS_SaveDeviceInvalid:
    case kS_SaveChooseDeviceInvalid:
    case kS_SaveFailed:
    case kS_SaveDisabledByCheat:
    case kS_LoadFailed:
        // Dialog/error states
        break;
    case kS_SaveDone:
        if (SongCacheNeedsWrite()) {
            SetState(kS_SaveSongCache);
        } else if (TheProfileMgr.GlobalOptionsNeedsSave()) {
            SetState(kS_SaveGlobalOptions);
        } else {
            SetState(kS_SaveCheckProfile);
        }
        break;
    case kS_SaveSongCache:
        TheSongMgr.StartSongCacheWrite();
        break;
    case kS_SaveGlobalOptions: {
        if (!mCacheID) {
            Symbol globalCacheName(kStrGlobalCacheName);
            mCacheID = TheCacheMgr->GetCacheID(globalCacheName);
        }
        if (!mCacheID) {
            SetState(kS_GlobalOptionsFailed);
        } else {
            SetState(kS_GlobalOptionsAllocRead);
        }
        break;
    }
    case kS_SaveCheckProfile:
        unk40 = GetAutosavableProfile();
        if (unk40) {
            if (TheMemcardMgr.IsStorageDeviceValid(unk40)) {
                SetState(kS_SaveOverwrite);
            } else {
                SetState(kS_SaveDeviceInvalid);
            }
        } else {
            SetState(kS_SaveCheckAutosave);
        }
        break;
    case kS_SaveCheckAutosave:
        TheProfileMgr.HandleProfileSaveComplete();
        SetState(kS_Done);
        break;
    case kS_ManualSaveInit:
        SetState(kS_ManualSaveChooseDevice);
        break;
    case kS_ManualSaveChooseDevice: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x7D6);
        mWaiting = true;
        TheMemcardMgr.SelectDevice(pProfile, this, unk3c, true);
        break;
    }
    case kS_ManualSaveNoDevice:
        // Dialog state
        break;
    case kS_ManualSaveDone:
        // Manual save complete
        break;
    case kS_ManualLoadInit: {
        int padNum = 0;
        if (unk40) {
            padNum = unk40->GetPadNum();
        }
        if (TheProfileMgr.HasUnsavedDataForPad(padNum)) {
            SetState(kS_ManualLoadConfirmUnsaved);
        } else {
            SetState(kS_ManualLoadConfirm);
        }
        break;
    }
    case kS_ManualLoadConfirmUnsaved:
    case kS_ManualLoadConfirm:
    case kS_ManualLoadNoDevice:
    case kS_ManualLoadMissing:
    case kS_ManualLoadNoFile:
    case kS_ManualLoadCorrupt:
    case kS_ManualLoadNotOwner:
        // Dialog states
        break;
    case kS_ManualLoadChooseDevice:
        // Choose device for manual load
        break;
    case kS_ManualLoadStartLoad: {
        HamProfile *pProfile = unk40;
        MILO_ASSERT(pProfile, 0x811);
        mWaiting = true;
        RELEASE(mAction);
        mAction = new LoadMemcardAction(pProfile);
        pProfile->PreLoad();
        TheMemcardMgr.OnLoadGame(pProfile, mAction);
        break;
    }
    case kS_ManualLoadDone:
        // Manual load complete
        break;
    case kS_Abort:
        // Abort state
        break;
    case kS_Done:
        TheMemcardMgr.SaveLoadAllComplete();
        Finish();
        break;
    default:
        break;
    }
}
