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
#include "utl/Cache.h"
#include "utl/CacheMgr.h"
#include "utl/Locale.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

namespace {
    const char *kStrGlobalCacheName = "global";
}

SaveLoadManager *TheSaveLoadMgr;

SaveLoadManager::SaveLoadManager()
    : mActivated(0), mFirstTime(1), mState(), mStateAtSelectStart(),
      mLastSelectionPadNum(-1), mProfileCur(0), mSongCacheSize(0), mHasAutoloaded(0),
      mCacheID(0), mCache(0), mData(0), mSongCacheWriteDisabled(0), mWaiting(0),
      mLastMCResult(), mLastCacheResult(), mNeedsSave(0), mNeedsLoad(0),
      mLastChosenDeviceID(0), mDeviceIDState(0), mAction(0) {
    SetName("saveload_mgr", ObjectDir::Main());
    ThePlatformMgr.AddSink(this, SigninChangedMsg::Type());
}

SaveLoadManager::~SaveLoadManager() {
    ThePlatformMgr.RemoveSink(this, SigninChangedMsg::Type());
    TheUIEventMgr->RemoveSink(this);
    RELEASE(mAction);
}

int SaveLoadManager::GetDialogFocusOption() {
    int ret = 0;
    if (mState == kS_ManualLoadConfirm) {
        ret = 1;
    }
    return ret;
}

void SaveLoadManager::HandleEventResponse(HamProfile *profile, int choiceIdx) {
    State start = mStateAtSelectStart;
    mStateAtSelectStart = kS_Idle;
    if (start != mState) {
        MILO_NOTIFY(
            "HandleEventResponse: expected state %d but am now in state %d\n",
            start,
            mState
        );
    } else if (choiceIdx >= 1 && choiceIdx <= 3) {
        if (profile) {
            mLastSelectionPadNum = profile->GetPadNum();
        } else {
            mLastSelectionPadNum = -1;
        }
        bool first = choiceIdx == 1;
        switch (mState) {
        case 6:
            if (choiceIdx == 1) {
                if (mDeviceIDState == 2) {
                    SetState((State)9);
                } else {
                    SetState((State)8);
                }
            } else {
                SetState((State)66);
            }
            break;
        case 0xC:
            SetState(first ? (State)13 : (State)66);
            break;
        case 7:
            SetState(first ? (State)10 : (State)66);
            break;
        case 0x17:
        case 0x18:
            SetState(first ? (State)25 : (State)36);
            break;
        case 0x1C:
            SetState(first ? (State)29 : (State)36);
            break;
        case 0x29:
        case 0x2A:
            SetState(first ? (State)43 : (State)54);
            break;
        case 0x2F:
            SetState(first ? (State)48 : (State)54);
            break;
        case 0x3A:
            SetState(first ? (State)59 : (State)64);
            break;
        case 0x42:
            switch (mMode) {
            case 0:
                SetState((State)3);
                break;
            case 1:
                SetState((State)0x54);
                break;
            case 2:
                SetState((State)0x55);
                break;
            default:
                break;
            }
            break;
        case 0xE:
        case 0xF:
        case 0x10:
        case 0x11:
        case 0x48:
            SetState(first ? (State)70 : (State)66);
            break;
        case 0x49:
        case 0x4e:
        case 0x4f:
        case 0x50:
        case 0x5f:
        case 0x61:
        case 0x62:
        case 0x63:
            SetState((State)0x42);
            break;
        case 0x4C:
            SetState(first ? (State)77 : (State)66);
            break;
        case 0x4A:
            switch (choiceIdx) {
            case 1:
                SetState((State)0x4B);
                break;
            case 2:
                SetState((State)0x47);
                break;
            default:
                SetState((State)0x42);
                break;
            }
            break;
        case 0x58:
            SetState(first ? (State)87 : (State)66);
            break;
        case 0x5b:
        case 0x5c:
            if (choiceIdx == 1) {
                SetState((State)0x5D);
            } else {
                SetState((State)0x44);
            }
            break;
        case 0x5E:
            SetState(first ? (State)93 : (State)66);
            break;
        default:
            MILO_FAIL(
                "Unhandled UIComponentSelectDoneMsg from choice index %i in state %d and mode %d",
                choiceIdx,
                (int)mState,
                (int)mMode
            );
            break;
        }
    } else {
        MILO_FAIL("Bad choice index %i", choiceIdx);
    }
}

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
    return mState == 0 && (!mActivated || (!mNeedsSave && !mNeedsLoad));
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
    return p || mFirstTime;
}

void SaveLoadManager::EnableAutosave(HamProfile *p) {
    if (!p) {
        MILO_NOTIFY("Tried to enable autosave without a valid profile.");
    } else {
        ManualSave(p);
    }
}

void SaveLoadManager::ManualSave(HamProfile *pProfile) {
    if (mState != 0) {
        State cur = mState;
        MILO_NOTIFY(
            "Attempted to perform a manual save, but saveloadmgr is not idle (state = %d).",
            cur
        );
    } else {
        MILO_ASSERT(pProfile, 0x364);
        mProfileCur = pProfile;
        mLastSelectionPadNum = pProfile->GetPadNum();
        TheMemcardMgr.AddSink(this);
        SetState((State)0x56);
    }
}

void SaveLoadManager::Start() {
    mLastSelectionPadNum = -1;
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
        return;
    } else if (!IsIdle()) {
        MILO_NOTIFY("Tried to disable autosave while saveloadmgr is not idle.");
        return;
    } else
        pProfile->SetSaveState(kMetaProfileError); // error?
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
    if (!mActivated) {
        mActivated = true;
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

DataNode SaveLoadManager::OnMsg(const NoDeviceChosenMsg &msg) {
    MILO_ASSERT(mWaiting, 0x8b9);
    mWaiting = false;
    switch (mState) {
    case kS_SaveChooseDeviceInvalid:
        SetState(kS_SaveDeviceInvalid);
        break;
    case kS_AutoloadSetDevice:
        SetState(kS_AutoloadNoSaveFound_Msg);
        break;
    case kS_AutoloadSelectDevice2:
        SetState(kS_AutoloadMultipleSavesFound);
        break;
    case kS_AutoloadSelectDevice3:
        SetState(kS_AutoloadDeviceMissing);
        break;
    case kS_ManualSaveChooseDevice:
        SetState(kS_ManualSaveNoDevice);
        break;
    case kS_ManualLoadChooseDevice:
        SetState(kS_ManualLoadNoDevice);
        break;
    case kS_Abort:
    case kS_Done:
    case kS_Finish:
        break;
    default:
        State state = mState;
        SaveLoadMode mode = mMode;
        MILO_FAIL("Unhandled NoDeviceChosenMsg in state %d and mode %d", state, mode);
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

DataNode SaveLoadManager::OnMsg(const SigninChangedMsg &msg) {
    static Symbol saveload_dialog_event("saveload_dialog_event");
    switch (mState) {
    case 0:
    case 0x65:
    case 0x66:
    case 0x67:
        break;
    case 6:
    case 7:
    case 0xc:
    case 0xe:
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x17:
    case 0x18:
    case 0x1c:
    case 0x29:
    case 0x2a:
    case 0x2f:
    case 0x3a:
    case 0x42:
    case 0x48:
    case 0x49:
    case 0x4a:
    case 0x4c:
    case 0x4e:
    case 0x4f:
    case 0x50:
    case 0x58:
    case 0x5b:
    case 0x5c:
    case 0x5e:
    case 0x5f:
    case 0x61:
    case 0x62:
    case 99: {
        HamProfile *critProfile = TheProfileMgr.CriticalProfile();
        bool changed = mProfileCur
            && ThePlatformMgr.HasPadNumsSigninChanged(mProfileCur->GetPadNum());

        bool critChanged = critProfile
            && ThePlatformMgr.HasPadNumsSigninChanged(critProfile->GetPadNum());
        if (changed) {
            if (TheUIEventMgr->HasActiveDialogEvent()
                && TheUIEventMgr->CurrentEvent() == saveload_dialog_event) {
                TheUIEventMgr->DismissEvent(gNullStr);
            } else {
                MILO_NOTIFY(
                    "Expected active dialog event during signin change on pad %d while in state %d.",
                    mProfileCur->GetPadNum(),
                    mState
                );
            }
            SetState((State)0x66);
        } else if (critChanged) {
            SetState((State)0x66);
        }
        break;
    }
    case 0xb:
    case 0x46:
    case 0x47:
    case 0x60:
        SetState((State)0x65);
        break;
    default:
        if (mProfileCur
            && ThePlatformMgr.HasPadNumsSigninChanged(mProfileCur->GetPadNum())) {
            MILO_NOTIFY(
                "SIGNOUT on pad %d not expected during state %d",
                mProfileCur->GetPadNum(),
                mState
            );
            SetState((State)0x65);
        }
        break;
    }
    return 0;
}

DataNode SaveLoadManager::OnMsg(const MCResultMsg &msg) {
    MILO_ASSERT(mWaiting, 0x8E8);
    mWaiting = false;
    MCResult res = msg.Result();
    switch (mState) {
    case 4:
    case 0x46:
    case 0x47:
        mLastMCResult = res;
        break;
    case 0xB:
        switch (res) {
        case 0:
            mLastMCResult = (MCResult)0;
            SetState((State)0x43);
            break;
        case 1:
            SetState((State)0xc);
            break;
        case 5:
            SetState((State)0xE);
            break;
        case 6:
        case 8:
            SetState((State)0x45);
            break;
        case 10:
            SetState((State)0x10);
            break;
        case 0xB:
            SetState((State)0x11);
            break;
        case 0x19:
            SetState((State)0xF);
            break;
        default:
            SetState((State)0x50);
            break;
        }
        break;
    case 0x45:
        switch (res) {
        case 0:
        case 5:
        case 7:
        case 0x19:
            SetState((State)0x48);
            break;
        case 1:
            SetState((State)0x4C);
            break;
        case 6:
        case 8:
            SetState((State)0x46);
            break;
        default:
            SetState((State)0x4E);
            break;
        }
        break;
    case 0x4B:
        SetState((State)0x47);
        break;
    case 0x60:
        switch (res) {
        case 0:
            mLastMCResult = (MCResult)0;
            SetState((State)0x43);
            break;
        case 1:
            SetState((State)0x5F);
            break;
        case 5:
            SetState((State)0x62);
            break;
        case 8:
            SetState((State)0x61);
            break;
        case 10:
            SetState((State)0x10);
            break;
        case 11:
            SetState((State)0x11);
            break;
        case 0x19:
            SetState((State)99);
            break;
        default:
            SetState((State)0x50);
            break;
        }
        break;
    case 0x65:
    case 0x66:
    case 0x67:
        break;
    default:
        MILO_FAIL(
            "Unhandled MCResultMsg in state %d and mode %d", (int)mState, (int)mMode
        );
        break;
    }
    return 0;
}

DataNode SaveLoadManager::GetDialogMsg() {
    String profileName = gNullStr;
    int playerNum = -1;
    if (mProfileCur) {
        profileName = mProfileCur->GetName();
        playerNum = mProfileCur->GetPadNum() + 1;
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
        HamProfile *pProfile = mProfileCur;
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

void SaveLoadManager::Poll() {
    if (mActivated) {
        if (mState == 0) {
            if (mNeedsSave && IsSafePlaceToSave()) {
                mMode = (SaveLoadMode)1;
                Start();
                mNeedsSave = false;
            } else if (mNeedsLoad && IsSafePlaceToLoad()) {
                mMode = (SaveLoadMode)0;
                Start();
                mNeedsLoad = false;
            } else if (!TheUIEventMgr->HasActiveDialogEvent()) {
                TheProfileMgr.PurgeOldData();
                if (IsReasonToAutoload()) {
                    mNeedsLoad = true;
                }
            }
        } else {
            switch (mState) {
            case 1:
                switch (mMode) {
                case 0:
                    SetState((State)2);
                    break;
                case 1:
                    SetState((State)0x51);
                    break;
                case 2:
                    SetState((State)0x42);
                    break;
                default:
                    MILO_NOTIFY("SaveLoadManager startup bad mode: %d", mMode);
                    SetState((State)0x66);
                    break;
                }
                break;
            case 4:
                if (!mWaiting) {
                    switch (mLastMCResult) {
                    case 7:
                        SetState((State)0xB);
                        break;
                    case 8:
                        SetState((State)5);
                        break;
                    case 9:
                        SetState((State)7);
                        break;
                    default:
                        SetState((State)0x42);
                        break;
                    }
                }
                break;
            case 0x14:
                if (TheCacheMgr->IsDone()) {
                    mLastCacheResult = TheCacheMgr->GetLastResult();
                    switch (mLastCacheResult) {
                    case 0:
                        TheCacheMgr->AddCacheID(mCacheID, mSongCacheName.c_str());
                        SetState((State)0x1B);
                        break;
                    case 6:
                        SetState((State)0x15);
                        break;
                    default:
                        MILO_NOTIFY(
                            "SaveLoadManager - CacheMgr search returned error %d",
                            (int)mLastCacheResult
                        );
                        SetState((State)0x25);
                        break;
                    }
                }
                break;
            case 0x19:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case 0:
                        mDeviceIDState = 2;
                        mLastChosenDeviceID = mCacheID->GetDeviceID();
                        TheCacheMgr->AddCacheID(mCacheID, mSongCacheName.c_str());
                        SetState((State)0x20);
                        break;
                    case 4:
                        mDeviceIDState = 1;
                        SetState((State)0x17);
                        break;
                    default:
                        MILO_FAIL(
                            "SaveLoadManager - CacheMgr choose returned error %d",
                            (int)res
                        );
                        SetState((State)0x25);
                        break;
                    }
                }
                break;
            case 0x1A:
                if (!ThePlatformMgr.GuideShowing()) {
                    SetState((State)0x19);
                }
                break;
            case 0x1B:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case 0:
                        SetState((State)0x1E);
                        break;
                    case 7:
                        SetState((State)0x1C);
                        break;
                    case 8:
                        SetState((State)0x16);
                        break;
                    default:
                        MILO_FAIL(
                            "SaveLoadManager - kS_SongCacheCreateMountRead unhandled error %d",
                            (int)res
                        );
                        SetState((State)0x25);
                        break;
                    }
                }
                break;
            case 0x20:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case 0:
                        SetState((State)0x21);
                        break;
                    case 7:
                        UpdateStatus((SaveLoadMgrStatus)2);
                        SetState((State)0x1C);
                        break;
                    case 8:
                        UpdateStatus((SaveLoadMgrStatus)2);
                        SetState((State)0x16);
                        break;
                    default:
                        UpdateStatus((SaveLoadMgrStatus)2);
                        MILO_FAIL(
                            "SaveLoadManager - kS_SongCacheCreateMountWrite unhandled error %d",
                            (int)res
                        );
                        SetState((State)0x25);
                        break;
                    }
                }
                break;
            case 0x1E:
                if (mCache->IsDone()) {
                    CacheResult res = mCache->GetLastResult();
                    switch (res) {
                    case 0:
                        SetState((State)0x1f);
                        break;
                    case 6:
                        SetState((State)0x21);
                        break;
                    case 8:
                        SetState((State)0x16);
                        break;
                    default:
                        SetState((State)0x25);
                        break;
                    }
                }
                break;
            case 0x1D:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus((SaveLoadMgrStatus)2);
                    SetState((State)0x20);
                }
                break;
            case 0x1F:
                if (mCache->IsDone()) {
                    CacheResult res = mCache->GetLastResult();
                    switch (res) {
                    case 0: {
                        BufStream stream(mData, mSongCacheSize, true);
                        TheSongMgr.LoadCachedSongInfo(stream);
                        SetState((State)0x22);
                        break;
                    }
                    default:
                        SetState((State)0x25);
                        break;
                    }
                }
                break;
            case 0x21:
            case 0x33:
            case 0x3e:
                if (mCache->IsDone()) {
                    mLastCacheResult = mCache->GetLastResult();
                    switch (mState) {
                    case 0x21:
                        SetState((State)0x23);
                        break;
                    case 0x33:
                        SetState((State)0x35);
                        break;
                    case 0x3E:
                        SetState((State)0x3F);
                        break;
                    default:
                        MILO_FAIL("Impossible state.");
                        break;
                    }
                }
                break;

            case 0x22:
                if (TheCacheMgr->IsDone()) {
                    if (TheCacheMgr->GetLastResult() == 0) {
                        SetState((State)0x26);
                    } else {
                        SetState((State)0x25);
                    }
                }
                break;
            case 0x23:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus((SaveLoadMgrStatus)2);
                    if (mLastCacheResult == 0) {
                        mLastCacheResult = TheCacheMgr->GetLastResult();
                    }
                    if (mLastCacheResult == 0) {
                        SetState((State)0x26);
                    } else {
                        SetState((State)0x25);
                    }
                }
                break;

            case 0x27:
                if (TheCacheMgr->IsDone()) {
                    mLastCacheResult = TheCacheMgr->GetLastResult();
                    switch (mLastCacheResult) {
                    case 0:
                        TheCacheMgr->AddCacheID(mCacheID, kStrGlobalCacheName);
                        SetState((State)0x2E);
                        break;
                    case 6:
                        switch (mDeviceIDState) {
                        case 0:
                            SetState((State)0x2B);
                            break;
                        case 2:
                            SetState((State)0x2C);
                            break;
                        default:
                            SetState((State)0x29);
                            break;
                        }
                        break;
                    default:
                        MILO_FAIL(
                            "SaveLoadManager - CacheMgr search returned error %d",
                            (int)mLastCacheResult
                        );
                        SetState((State)0x37);
                        break;
                    }
                }
                break;

            case 0x2B:
            case 0x2C:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case 0:
                        mDeviceIDState = 2;
                        mLastChosenDeviceID = mCacheID->GetDeviceID();
                        TheCacheMgr->AddCacheID(mCacheID, kStrGlobalCacheName);
                        SetState((State)0x31);
                        break;
                    case 4:
                        mDeviceIDState = 1;
                        SetState((State)0x29);
                        break;
                    default:
                        MILO_FAIL(
                            "SaveLoadManager - CacheMgr choose returned error %d",
                            (int)res
                        );
                        SetState((State)0x37);
                        break;
                    }
                }
                break;
            case 0x2D:
                if (!ThePlatformMgr.GuideShowing()) {
                    SetState((State)0x2B);
                }
                break;
            case 0x2E:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case 0:
                        SetState((State)0x32);
                        break;
                    case 7:
                        SetState((State)0x2F);
                        break;
                    case 8:
                        SetState((State)0x28);
                        break;
                    default:
                        MILO_NOTIFY(
                            "SaveLoadManager - unknown error %d during state %d.",
                            (int)res,
                            (int)mState
                        );
                        SetState((State)0x37);
                        break;
                    }
                }
                break;
            case 0x31:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case 0:
                        SetState((State)0x33);
                        break;
                    case 7:
                        UpdateStatus((SaveLoadMgrStatus)2);
                        SetState((State)0x2F);
                        break;
                    case 8:
                        UpdateStatus((SaveLoadMgrStatus)2);
                        SetState((State)0x28);
                        break;
                    default:
                        UpdateStatus((SaveLoadMgrStatus)2);
                        MILO_NOTIFY(
                            "SaveLoadManager - unknown error %d during state %d.",
                            (int)res,
                            (int)mState
                        );
                        SetState((State)0x37);
                        break;
                    }
                }
                break;
            case 0x30:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus((SaveLoadMgrStatus)2);
                    SetState((State)0x31);
                }
                break;
            case 0x32:
                if (mCache->IsDone()) {
                    if (mCache->GetLastResult() == 0) {
                        int size = TheProfileMgr.GetGlobalOptionsSize();
                        FixedSizeSaveableStream stream(mData, size, true);
                        TheProfileMgr.LoadGlobalOptions(stream);
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)1);
                    } else {
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
                    }
                    SetState((State)0x34);
                }
                break;
            case 0x3B:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case 0:
                        mDeviceIDState = 2;
                        mLastChosenDeviceID = mCacheID->GetDeviceID();
                        TheCacheMgr->AddCacheID(mCacheID, kStrGlobalCacheName);
                        SetState((State)0x3D);
                        break;
                    case 4:
                        mDeviceIDState = 1;
                        SetState((State)0x3A);
                        break;
                    default:
                        MILO_NOTIFY(
                            "SaveLoadManager - CacheMgr choose returned error %d",
                            (int)res
                        );
                        SetState((State)0x40);
                        break;
                    }
                }
                break;
            case 0x3C:
                if (!ThePlatformMgr.GuideShowing()) {
                    SetState((State)0x3B);
                }
                break;
            case 0x3D:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case 0:
                        SetState((State)0x3E);
                        break;
                    case 8:
                        UpdateStatus((SaveLoadMgrStatus)2);
                        SetState((State)0x3A);
                        break;
                    default:
                        UpdateStatus((SaveLoadMgrStatus)2);
                        MILO_FAIL(
                            "SaveLoadManager - CacheMgr choose returned error %d",
                            (int)res
                        );
                        SetState((State)0x40);
                        break;
                    }
                }
                break;

            case 0x34:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    if (res == 0) {
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)1);
                        SetState((State)0x38);
                    } else {
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
                        SetState((State)0x38);
                    }
                }
                break;
            case 0x35:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus((SaveLoadMgrStatus)2);
                    if (mLastCacheResult == 0) {
                        mLastCacheResult = TheCacheMgr->GetLastResult();
                    }
                    if (mLastCacheResult == 0) {
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)1);
                        SetState((State)0x38);
                    } else {
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
                        SetState((State)0x38);
                    }
                }
                break;
            case 0x3F:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus((SaveLoadMgrStatus)2);
                    if (mLastCacheResult == 0) {
                        mLastCacheResult = TheCacheMgr->GetLastResult();
                    }
                    if (mLastCacheResult == 0) {
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)1);
                        SetState((State)0x41);
                    } else {
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
                        SetState((State)0x41);
                    }
                }
                break;

            case 0x46:
            case 0x47:
                if (!mWaiting) {
                    UpdateStatus((SaveLoadMgrStatus)2);
                    switch (mLastMCResult) {
                    case 0:
                        SetState((State)0x43);
                        break;
                    case 1:
                        SetState((State)0x4C);
                        break;
                    case 6:
                        mDeviceIDState = 0;
                        mLastChosenDeviceID = 0;
                        SetState((State)0x49);
                        break;
                    case 7:
                        MILO_ASSERT(mState != kS_SaveOverwrite, 0x2ED);
                        SetState((State)0x48);
                        break;
                    default:
                        SetState((State)0x4E);
                        break;
                    }
                }
                break;

            case 0x52:
                if (TheSongMgr.IsSongCacheWriteDone()) {
                    if (TheProfileMgr.GlobalOptionsNeedsSave()) {
                        SetState((State)0x53);
                    } else {
                        SetState((State)0x54);
                    }
                }
                break;

            case 0x65:
            case 0x67:
                if (!mWaiting) {
                    if (mCache) {
                        if (mCache->IsDone()) {
                            TheCacheMgr->UnmountAsync(&mCache, nullptr);
                        }
                    } else if (TheCacheMgr->IsDone()) {
                        if (mState == 0x65) {
                            SetState((State)0x66);
                        } else {
                            SetState((State)0);
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}

void SaveLoadManager::SetState(State newState) {
    if (mState != newState) {
        static Symbol saveload_dialog_event("saveload_dialog_event");
        bool b14 = false;
        switch (mState) {
        case 0:
            b14 = true;
            break;
        case 0xB:
        case 0x46:
        case 0x47:
        case 0x60:
            if (newState != 0x65) {
                RELEASE(mAction);
            }
            break;
        case 0x1F:
        case 0x21:
        case 0x32:
        case 0x33:
        case 0x3E:
            if (newState != 0x67) {
                if (mData) {
                    MemFree(mData, __FILE__, 0x424);
                    mData = nullptr;
                }
            }
            break;
        case 0x65:
            RELEASE(mAction);
            break;
        case 0x67:
            if (mData) {
                MemFree(mData, __FILE__, 0x433);
                mData = nullptr;
            }
            break;
        default:
            break;
        }
        mState = newState;
        if (b14) {
            UpdateStatus((SaveLoadMgrStatus)0);
        }
        switch (mState) {
        case 0:
            UpdateStatus((SaveLoadMgrStatus)5);
            break;
        case 1:
            mDeviceIDState = 0;
            break;
        case 2:
            if (mFirstTime) {
                SetState((State)0x14);
            } else {
                SetState((State)3);
            }
            break;
        case 3:
            mProfileCur = GetNewSigninProfile();
            if (!mProfileCur) {
                SetState((State)0x12);
            } else {
                SetState((State)4);
            }
            break;
        case 4: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x48B);
            mWaiting = true;
            TheMemcardMgr.OnSearchForDevice(pProfile);
            break;
        }
        case 5:
            if (mDeviceIDState == 2) {
                SetState((State)9);
            } else {
                SetState((State)6);
            }
            break;
        case 9:
            MILO_ASSERT(mDeviceIDState == kDeviceID_Chosen, 0x4AB);
            TheMemcardMgr.SetDevice(mLastChosenDeviceID);
            SetState((State)0xB);
            break;
        case 8: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x4B6);
            mWaiting = true;
            TheMemcardMgr.SelectDevice(pProfile, this, mLastSelectionPadNum, false);
            break;
        }
        case 10:
        case 0xd:
        case 0x4d: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x4C7);
            mWaiting = true;
            TheMemcardMgr.SelectDevice(pProfile, this, mLastSelectionPadNum, false);
            break;
        }
        case 0xB: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x4D6);
            mWaiting = true;
            RELEASE(mAction);
            mAction = new LoadMemcardAction(pProfile);
            pProfile->PreLoad();
            TheMemcardMgr.OnLoadGame(pProfile, mAction);
            break;
        }
        case 0x12:
            mFirstTime = false;
            if (TheProfileMgr.GlobalOptionsNeedsSave()) {
                SetState((State)0x13);
            } else {
                TheProfileMgr.HandleProfileLoadComplete();
                SetState((State)0x66);
            }
            break;
        case 0x59:
            SetState((State)0x66);
            break;
        case 0x14:
            mSongCacheName = TheSongMgr.GetCachedSongInfoName();
            if (mCacheID) {
                TheCacheMgr->RemoveCacheID(mCacheID);
                RELEASE(mCacheID);
            }
            if (!TheCacheMgr->SearchAsync(mSongCacheName.c_str(), &mCacheID)) {
                MILO_FAIL(
                    "TheCacheMgr->SearchAsync() failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case 0x15:
        case 0x16:
            SetState((State)0x19);
            break;
        case 0x19: {
            if (mCacheID) {
                TheCacheMgr->RemoveCacheID(mCacheID);
                RELEASE(mCacheID);
            }
            static Symbol song_info_cache_name("song_info_cache_name");
            bool *temp = nullptr;
            if (!TheCacheMgr->ShowUserSelectUIAsync(
                    nullptr,
                    0x25800,
                    mSongCacheName.c_str(),
                    Localize(song_info_cache_name, temp, TheLocale),
                    &mCacheID
                )) {
                if (TheCacheMgr->GetLastResult() != 0) {
                    SetState((State)0x1A);
                }
            }
            break;
        }
        case 0x1B:
        case 0x2E:
            if (!TheCacheMgr->MountAsync(mCacheID, &mCache, nullptr)) {
                MILO_FAIL(
                    "TheCacheMgr->MountAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;

        case 0x20:
        case 0x31:
        case 0x3d:
            UpdateStatus((SaveLoadMgrStatus)1);
            if (!TheCacheMgr->MountAsync(mCacheID, &mCache, nullptr)) {
                MILO_FAIL(
                    "TheCacheMgr->MountAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case 0x1D:
        case 0x30:
            UpdateStatus((SaveLoadMgrStatus)1);
            if (!TheCacheMgr->DeleteAsync(mCacheID)) {
                MILO_FAIL(
                    "TheCacheMgr->DeleteAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case 0x1F:
            mData = _MemAllocTemp(mSongCacheSize, __FILE__, 0x578, "SaveLoadManager", 0);
            if (!mCache->ReadAsync(
                    mSongCacheName.c_str(), mData, mSongCacheSize, nullptr
                )) {
                MILO_FAIL(
                    "mCache->ReadAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case 0x1E:
            if (!mCache->GetFileSizeAsync(
                    mSongCacheName.c_str(), &mSongCacheSize, nullptr
                )) {
                MILO_FAIL(
                    "mCache->GetFileSizeAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case 0x21: {
            int size = TheSongMgr.GetCachedSongInfoSize();
            mData = _MemAllocTemp(size, __FILE__, 0x595, "SaveLoadManager", 0);
            BufStream stream(mData, size, true);
            if (TheSongMgr.SaveCachedSongInfo(stream)
                && !mCache->WriteAsync(mSongCacheName.c_str(), mData, size, nullptr)) {
                MILO_FAIL(
                    "mCache->WriteAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        }
        case 0x22:
        case 0x23:
            if (!TheCacheMgr->UnmountAsync(&mCache, nullptr)) {
                MILO_FAIL(
                    "TheCacheMgr->UnmountAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case 0x24:
            mDeviceIDState = 1;
            mLastChosenDeviceID = 0;
            mSongCacheWriteDisabled = true;
            SetState(mCache ? (State)34 : (State)38);
            break;
        case 0x25:
            mDeviceIDState = 0;
            mLastChosenDeviceID = 0;
            mSongCacheWriteDisabled = true;
            SetState(mCache ? (State)34 : (State)38);
            break;
        case 0x26:
            mCacheID = nullptr;
            SetState((State)0x27);
            break;

        case 0x13:
            if (!mCacheID) {
                mCacheID = TheCacheMgr->GetCacheID(kStrGlobalCacheName);
            }
            if (!mCacheID) {
                SetState((State)0x37);
            } else {
                SetState((State)0x31);
            }
            break;
        case 0x27:
            if (mCacheID) {
                TheCacheMgr->RemoveCacheID(mCacheID);
                RELEASE(mCacheID);
            }
            if (!TheCacheMgr->SearchAsync(kStrGlobalCacheName, &mCacheID)) {
                MILO_FAIL(
                    "TheCacheMgr->SearchAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case 0x28:
            if (mDeviceIDState != 0) {
                mDeviceIDState = 0;
                SetState((State)0x2A);
            } else {
                SetState((State)0x2B);
            }
            break;
        case 0x2B: {
            if (mCacheID) {
                TheCacheMgr->RemoveCacheID(mCacheID);
                RELEASE(mCacheID);
            }
            static Symbol global_options_cache_name("global_options_cache_name");
            int size = TheProfileMgr.GetGlobalOptionsSize();
            bool *temp = nullptr;
            if (!TheCacheMgr->ShowUserSelectUIAsync(
                    nullptr,
                    size,
                    kStrGlobalCacheName,
                    Localize(global_options_cache_name, temp, TheLocale),
                    &mCacheID
                )) {
                if (TheCacheMgr->GetLastResult() != 0) {
                    SetState((State)0x2D);
                }
            }
            break;
        }
        case 0x2C: {
            MILO_ASSERT(mDeviceIDState == kDeviceID_Chosen, 0x627);
            if (mCacheID) {
                TheCacheMgr->RemoveCacheID(mCacheID);
                RELEASE(mCacheID);
            }
            static Symbol global_options_cache_name("global_options_cache_name");
            TheCacheMgr->CreateCacheIDFromDeviceID(
                mLastChosenDeviceID,
                kStrGlobalCacheName,
                Localize(global_options_cache_name, nullptr, TheLocale),
                &mCacheID
            );
            break;
        }
        case 0x39:
            if (mDeviceIDState == 0 || mDeviceIDState == 2) {
                SetState((State)0x3B);
            } else {
                SetState((State)0x3A);
            }
            break;
        case 0x3B: {
            if (mCacheID) {
                TheCacheMgr->RemoveCacheID(mCacheID);
                RELEASE(mCacheID);
            }
            static Symbol global_options_cache_name("global_options_cache_name");
            int size = TheProfileMgr.GetGlobalOptionsSize();
            bool *temp = nullptr;
            if (!TheCacheMgr->ShowUserSelectUIAsync(
                    nullptr,
                    size,
                    kStrGlobalCacheName,
                    Localize(global_options_cache_name, temp, TheLocale),
                    &mCacheID
                )) {
                if (TheCacheMgr->GetLastResult() != 0) {
                    SetState((State)0x3C);
                }
            }
            break;
        }
        case 0x32: {
            int size = TheProfileMgr.GetGlobalOptionsSize();
            mData = _MemAllocTemp(size, __FILE__, 0x69B, "SaveLoadManager", 0);
            if (!mCache->ReadAsync(kStrGlobalCacheName, mData, size, nullptr)) {
                MILO_FAIL(
                    "TheCacheMgr->ReadAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        }
        case 0x33:
        case 0x3E: {
            UpdateStatus((SaveLoadMgrStatus)1);
            int size = TheProfileMgr.GetGlobalOptionsSize();
            mData = _MemAllocTemp(size, __FILE__, 0x6AD, "SaveLoadManager", 0);
            FixedSizeSaveableStream stream(mData, size, true);
            TheProfileMgr.SaveGlobalOptions(stream);
            if (!mCache->WriteAsync(kStrGlobalCacheName, mData, size, nullptr)) {
                MILO_FAIL(
                    "mCache->WriteAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        }
        case 0x34:
        case 0x35:
        case 0x3F:
            if (!TheCacheMgr->UnmountAsync(&mCache, nullptr)) {
                if (TheCacheMgr->GetLastResult() != 8) {
                    MILO_NOTIFY(
                        "UnmountAsync failed with error %d",
                        (int)TheCacheMgr->GetLastResult()
                    );
                }
            }
            break;
        case 0x36:
            mDeviceIDState = 1;
            mLastChosenDeviceID = 0;
            TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
            SetState((State)0x38);
            break;
        case 0x37:
            mDeviceIDState = 0;
            mLastChosenDeviceID = 0;
            TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
            SetState((State)0x38);
            break;
        case 0x38:
            if (TheProfileMgr.GetNewlySignedIn().size() > 1) {
                mDeviceIDState = 1;
            }
            SetState((State)3);
            break;
        case 0x40:
            mDeviceIDState = 0;
            mLastChosenDeviceID = 0;
            TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
            SetState((State)0x41);
            break;
        case 0x42: {
            mDeviceIDState = 0;
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x6FE);
            TheMemcardMgr.SaveLoadProfileComplete(pProfile, 2);
            TheUIEventMgr->TriggerEvent(saveload_dialog_event, nullptr);
            break;
        }
        case 6:
        case 7:
        case 0xc:
        case 0xe:
        case 0xf:
        case 0x10:
        case 0x11:
        case 0x17:
        case 0x18:
        case 0x1c:
        case 0x29:
        case 0x2a:
        case 0x2f:
        case 0x3a:
        case 0x48:
        case 0x49:
        case 0x4a:
        case 0x4c:
        case 0x4e:
        case 0x4f:
        case 0x50:
        case 0x58:
        case 0x5b:
        case 0x5c:
        case 0x5e:
        case 0x5f:
        case 0x61:
        case 0x62:
        case 99:
            TheUIEventMgr->TriggerEvent(saveload_dialog_event, nullptr);
            break;
        case 0x43:
        case 0x44: {
            mDeviceIDState = 0;
            int i16 = mState != 0x43 ? -1 : 1;
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x713);
            TheMemcardMgr.SaveLoadProfileComplete(pProfile, i16);
            switch (mMode) {
            case 0:
                SetState((State)3);
                break;
            case 1:
                SetState((State)0x54);
                break;
            default:
                break;
            }
            break;
        }
        case 0x41:
            SetState((State)0x54);
            break;
        case 0x45: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x72d);
            mWaiting = true;
            TheMemcardMgr.OnCheckForSaveContainer(pProfile);
            break;
        }
        case 0x46: {
            UpdateStatus((SaveLoadMgrStatus)1);
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x739);
            mWaiting = true;
            RELEASE(mAction);
            mAction = new SaveMemcardAction(pProfile);
            TheMemcardMgr.OnSaveGame(pProfile, mAction, 1);
            break;
        }
        case 0x47: {
            UpdateStatus((SaveLoadMgrStatus)1);
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x747);
            mWaiting = true;
            RELEASE(mAction);
            mAction = new SaveMemcardAction(pProfile);
            TheMemcardMgr.OnSaveGame(pProfile, mAction, 0);
            break;
        }
        case 0x4B: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x76d);
            mWaiting = true;
            TheMemcardMgr.OnDeleteSaves(pProfile);
            break;
        }
        case 0x51:
            if (SongCacheNeedsWrite()) {
                SetState((State)0x52);
            } else if (TheProfileMgr.GlobalOptionsNeedsSave()) {
                SetState((State)0x53);
            } else {
                SetState((State)0x54);
            }
            break;
        case 0x52:
            TheSongMgr.StartSongCacheWrite();
            break;
        case 0x53:
            if (!mCacheID) {
                mCacheID = TheCacheMgr->GetCacheID(kStrGlobalCacheName);
            }
            if (!mCacheID) {
                SetState((State)0x40);
            } else {
                SetState((State)0x3d);
            }
            break;
        case 0x54:
            mProfileCur = GetAutosavableProfile();
            if (mProfileCur) {
                if (TheMemcardMgr.IsStorageDeviceValid(mProfileCur)) {
                    SetState((State)0x46);
                } else {
                    SetState((State)0x4c);
                }
            } else {
                SetState((State)0x55);
            }
            break;
        case 0x55:
            TheProfileMgr.HandleProfileSaveComplete();
            SetState((State)0x66);
            break;
        case 0x56:
            SetState((State)0x57);
            break;
        case 0x57:
        case 0x5D: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x7d6);
            mWaiting = true;
            TheMemcardMgr.SelectDevice(pProfile, this, mLastSelectionPadNum, true);
            break;
        }
        case 0x5A: {
            int pad = 0;
            if (mProfileCur) {
                pad = mProfileCur->GetPadNum();
            }
            if (TheProfileMgr.HasUnsavedDataForPad(pad)) {
                SetState((State)0x5B);
            } else {
                SetState((State)0x5C);
            }
            break;
        }
        case 0x60: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x811);
            mWaiting = true;
            RELEASE(mAction);
            mAction = new LoadMemcardAction(pProfile);
            pProfile->PreLoad();
            TheMemcardMgr.OnLoadGame(pProfile, mAction);
            break;
        }
        case 0x66:
            TheMemcardMgr.SaveLoadAllComplete();
            Finish();
            break;
        default:
            break;
        }
    }
}
