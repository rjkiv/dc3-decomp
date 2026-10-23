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
      mLastChosenDeviceID(0), mDeviceIDState(), mAction(0) {
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
    mStateAtSelectStart = kS_None;
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
        case kS_AutoloadNoSaveFound_Msg:
            if (choiceIdx == 1) {
                if (mDeviceIDState == kDeviceID_Chosen) {
                    SetState(kS_AutoloadChooseDeviceManual);
                } else {
                    SetState(kS_AutoloadChooseDeviceNone);
                }
            } else {
                SetState(kS_SaveLoadError);
            }
            break;
        case kS_AutoloadDeviceMissing:
            SetState(first ? kS_AutoloadChooseDeviceMissing : kS_SaveLoadError);
            break;
        case kS_AutoloadMultipleSavesFound:
            SetState(first ? kS_AutoloadChooseDeviceMulti : kS_SaveLoadError);
            break;
        case kS_SongCacheCreateNotFound_Msg:
        case kS_SongCacheCreateMissing_Msg:
            SetState(first ? kS_SongCacheCreateChoose : kS_SongCacheCreateDecline);
            break;
        case kS_SongCacheCreateCorrupt:
            SetState(first ? kS_SongCacheCreateMountDelete : kS_SongCacheCreateDecline);
            break;
        case kS_GlobalCreateNotFound_Msg:
        case kS_GlobalCreateMissing_Msg:
            SetState(first ? kS_GlobalCreateChoose : kS_GlobalCreateDecline);
            break;
        case kS_GlobalCreateCorrupt:
            SetState(first ? kS_GlobalCreateMountDelete : kS_GlobalCreateDecline);
            break;
        case kS_GlobalOptionsMissing_Msg:
            SetState(first ? kS_GlobalOptionsChoose : kS_GlobalOptionsError);
            break;
        case kS_SaveLoadError:
            switch (mMode) {
            case kAutoLoad:
                SetState(kS_AutoloadFindNext);
                break;
            case kAutoSave:
                SetState(kS_AutosaveFindNext);
                break;
            case kDisableAutoSave:
                SetState(kS_AutosaveAllDone);
                break;
            default:
                break;
            }
            break;
        case kS_AutoloadCorrupt:
        case kS_AutoloadNotOwner:
        case kS_AutoloadObsolete:
        case kS_AutoloadFuture:
        case kS_SaveConfirmOverwrite:
            SetState(first ? kS_SaveOverwrite : kS_SaveLoadError);
            break;
        case kS_SaveNotEnoughSpace:
        case kS_SaveFailed:
        case kS_SaveDisabledByCheat:
        case kS_LoadFailed:
        case kS_ManualLoadMissing:
        case kS_ManualLoadNoFile:
        case kS_ManualLoadCorrupt:
        case kS_ManualLoadNotOwner:
            SetState(kS_SaveLoadError);
            break;
        case kS_SaveDeviceInvalid:
            SetState(first ? kS_SaveChooseDeviceInvalid : kS_SaveLoadError);
            break;
        case kS_SaveNotEnoughSpacePS3:
            switch (choiceIdx) {
            case 1:
                SetState(kS_SaveDeleteSavesPS3);
                break;
            case 2:
                SetState(kS_SaveNoOverwrite);
                break;
            default:
                SetState(kS_SaveLoadError);
                break;
            }
            break;
        case kS_ManualSaveNoDevice:
            SetState(first ? kS_ManualSaveChooseDevice : kS_SaveLoadError);
            break;
        case kS_ManualLoadConfirmUnsaved:
        case kS_ManualLoadConfirm:
            if (choiceIdx == 1) {
                SetState(kS_ManualLoadChooseDevice);
            } else {
                SetState(kS_SaveLoadUnchanged);
            }
            break;
        case kS_ManualLoadNoDevice:
            SetState(first ? kS_ManualLoadChooseDevice : kS_SaveLoadError);
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

FORCEINLINE bool SaveLoadManager::IsIdle() const {
    return mState == kS_None && (!mActivated || (!mNeedsSave && !mNeedsLoad));
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
    if (mState != kS_None) {
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
        SetState(kS_ManualSaveInit);
    }
}

void SaveLoadManager::Start() {
    mLastSelectionPadNum = -1;
    TheMemcardMgr.AddSink(this);
    SetState(kS_Startup);
    if (mMode == kAutoLoad) {
        UpdateStatus(kStatus_AutoloadStart);
    }
}

void SaveLoadManager::Finish() {
    if (mMode == kAutoLoad) {
        UpdateStatus(kStatus_AutoloadEnd);
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
        pProfile->SetSaveState(kMetaProfileError);
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
    case kS_AutoloadChooseDeviceNone:
    case kS_AutoloadChooseDeviceManual:
    case kS_AutoloadChooseDeviceMulti:
    case kS_AutoloadChooseDeviceMissing:
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
    case kS_AutoloadChooseDeviceNone:
        SetState(kS_AutoloadNoSaveFound_Msg);
        break;
    case kS_AutoloadChooseDeviceMulti:
        SetState(kS_AutoloadMultipleSavesFound);
        break;
    case kS_AutoloadChooseDeviceMissing:
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
    case kS_None:
    case kS_Abort:
    case kS_Done:
    case kS_Finish:
        break;
    case kS_AutoloadNoSaveFound_Msg:
    case kS_AutoloadMultipleSavesFound:
    case kS_AutoloadDeviceMissing:
    case kS_AutoloadCorrupt:
    case kS_AutoloadNotOwner:
    case kS_AutoloadObsolete:
    case kS_AutoloadFuture:
    case kS_SongCacheCreateNotFound_Msg:
    case kS_SongCacheCreateMissing_Msg:
    case kS_SongCacheCreateCorrupt:
    case kS_GlobalCreateNotFound_Msg:
    case kS_GlobalCreateMissing_Msg:
    case kS_GlobalCreateCorrupt:
    case kS_GlobalOptionsMissing_Msg:
    case kS_SaveLoadError:
    case kS_SaveConfirmOverwrite:
    case kS_SaveNotEnoughSpace:
    case kS_SaveNotEnoughSpacePS3:
    case kS_SaveDeviceInvalid:
    case kS_SaveFailed:
    case kS_SaveDisabledByCheat:
    case kS_LoadFailed:
    case kS_ManualSaveNoDevice:
    case kS_ManualLoadConfirmUnsaved:
    case kS_ManualLoadConfirm:
    case kS_ManualLoadNoDevice:
    case kS_ManualLoadMissing:
    case kS_ManualLoadNoFile:
    case kS_ManualLoadCorrupt:
    case kS_ManualLoadNotOwner: {
        HamProfile *critProfile = TheProfileMgr.GetCriticalProfile();
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
            SetState(kS_Done);
        } else if (critChanged) {
            SetState(kS_Done);
        }
        break;
    }
    case kS_AutoloadStartLoad:
    case kS_SaveOverwrite:
    case kS_SaveNoOverwrite:
    case kS_ManualLoadStartLoad:
        SetState(kS_Abort);
        break;
    default:
        if (mProfileCur
            && ThePlatformMgr.HasPadNumsSigninChanged(mProfileCur->GetPadNum())) {
            MILO_NOTIFY(
                "SIGNOUT on pad %d not expected during state %d",
                mProfileCur->GetPadNum(),
                mState
            );
            SetState(kS_Abort);
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
    case kS_AutoloadSearch:
    case kS_SaveOverwrite:
    case kS_SaveNoOverwrite:
        mLastMCResult = res;
        break;
    case kS_AutoloadStartLoad:
        switch (res) {
        case kMCNoError:
            mLastMCResult = kMCNoError;
            SetState(kS_SaveLoadSuccess);
            break;
        case kMCNoCard:
            SetState(kS_AutoloadDeviceMissing);
            break;
        case kMCCorrupt:
            SetState(kS_AutoloadCorrupt);
            break;
        case kMCNotEnoughSpace:
        case kMCFileNotFound:
            SetState(kS_SaveLookForFile);
            break;
        case kMCObsoleteVersion:
            SetState(kS_AutoloadObsolete);
            break;
        case kMCNewerVersion:
            SetState(kS_AutoloadFuture);
            break;
        case kMCNotOwner:
            SetState(kS_AutoloadNotOwner);
            break;
        default:
            SetState(kS_LoadFailed);
            break;
        }
        break;
    case kS_SaveLookForFile:
        switch (res) {
        case kMCNoError:
        case kMCCorrupt:
        case kMCFileExists:
        case kMCNotOwner:
            SetState(kS_SaveConfirmOverwrite);
            break;
        case kMCNoCard:
            SetState(kS_SaveDeviceInvalid);
            break;
        case kMCNotEnoughSpace:
        case kMCFileNotFound:
            SetState(kS_SaveOverwrite);
            break;
        default:
            SetState(kS_SaveFailed);
            break;
        }
        break;
    case kS_SaveDeleteSavesPS3:
        SetState(kS_SaveNoOverwrite);
        break;
    case kS_ManualLoadStartLoad:
        switch (res) {
        case kMCNoError:
            mLastMCResult = kMCNoError;
            SetState(kS_SaveLoadSuccess);
            break;
        case kMCNoCard:
            SetState(kS_ManualLoadMissing);
            break;
        case kMCCorrupt:
            SetState(kS_ManualLoadCorrupt);
            break;
        case kMCFileNotFound:
            SetState(kS_ManualLoadNoFile);
            break;
        case kMCObsoleteVersion:
            SetState(kS_AutoloadObsolete);
            break;
        case kMCNewerVersion:
            SetState(kS_AutoloadFuture);
            break;
        case kMCNotOwner:
            SetState(kS_ManualLoadNotOwner);
            break;
        default:
            SetState(kS_LoadFailed);
            break;
        }
        break;
    case kS_Abort:
    case kS_Done:
    case kS_Finish:
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
        if (mState == kS_None) {
            if (mNeedsSave && IsSafePlaceToSave()) {
                mMode = kAutoSave;
                Start();
                mNeedsSave = false;
            } else if (mNeedsLoad && IsSafePlaceToLoad()) {
                mMode = kAutoLoad;
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
            case kS_Startup:
                switch (mMode) {
                case kAutoLoad:
                    SetState(kS_AutoloadInit);
                    break;
                case kAutoSave:
                    SetState(kS_AutosaveInit);
                    break;
                case kDisableAutoSave:
                    SetState(kS_SaveLoadError);
                    break;
                default:
                    MILO_NOTIFY("SaveLoadManager startup bad mode: %d", mMode);
                    SetState(kS_Done);
                    break;
                }
                break;
            case kS_AutoloadSearch:
                if (!mWaiting) {
                    switch (mLastMCResult) {
                    case kMCFileExists:
                        SetState(kS_AutoloadStartLoad);
                        break;
                    case kMCFileNotFound:
                        SetState(kS_AutoloadNoSaveFound);
                        break;
                    case kMCMultipleFilesFound:
                        SetState(kS_AutoloadMultipleSavesFound);
                        break;
                    default:
                        SetState(kS_SaveLoadError);
                        break;
                    }
                }
                break;
            case kS_SongCacheCreateSearch:
                if (TheCacheMgr->IsDone()) {
                    mLastCacheResult = TheCacheMgr->GetLastResult();
                    switch (mLastCacheResult) {
                    case kCache_NoError:
                        TheCacheMgr->AddCacheID(mCacheID, mSongCacheName.c_str());
                        SetState(kS_SongCacheCreateMountRead);
                        break;
                    case kCache_ErrorCacheNotFound:
                        SetState(kS_SongCacheCreateNotFound);
                        break;
                    default:
                        MILO_NOTIFY(
                            "SaveLoadManager - CacheMgr search returned error %d",
                            (int)mLastCacheResult
                        );
                        SetState(kS_SongCacheCreateError);
                        break;
                    }
                }
                break;
            case kS_SongCacheCreateChoose:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        mDeviceIDState = kDeviceID_Chosen;
                        mLastChosenDeviceID = mCacheID->GetDeviceID();
                        TheCacheMgr->AddCacheID(mCacheID, mSongCacheName.c_str());
                        SetState(kS_SongCacheCreateMountWrite);
                        break;
                    case kCache_ErrorUserCancel:
                        mDeviceIDState = kDeviceID_Declined;
                        SetState(kS_SongCacheCreateNotFound_Msg);
                        break;
                    default:
                        MILO_FAIL(
                            "SaveLoadManager - CacheMgr choose returned error %d",
                            (int)res
                        );
                        SetState(kS_SongCacheCreateError);
                        break;
                    }
                }
                break;
            case kS_SongCacheCreateChooseWaitOnGuide:
                if (!ThePlatformMgr.GuideShowing()) {
                    SetState(kS_SongCacheCreateChoose);
                }
                break;
            case kS_SongCacheCreateMountRead:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        SetState(kS_SongCacheCreateFileSize);
                        break;
                    case kCache_ErrorCorrupt:
                        SetState(kS_SongCacheCreateCorrupt);
                        break;
                    case kCache_ErrorStorageDeviceMissing:
                        SetState(kS_SongCacheCreateMissing);
                        break;
                    default:
                        MILO_FAIL(
                            "SaveLoadManager - kS_SongCacheCreateMountRead unhandled error %d",
                            (int)res
                        );
                        SetState(kS_SongCacheCreateError);
                        break;
                    }
                }
                break;
            case kS_SongCacheCreateMountWrite:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        SetState(kS_SongCacheCreateWrite);
                        break;
                    case kCache_ErrorCorrupt:
                        UpdateStatus(kStatus_PhysicalWriteEnd);
                        SetState(kS_SongCacheCreateCorrupt);
                        break;
                    case kCache_ErrorStorageDeviceMissing:
                        UpdateStatus(kStatus_PhysicalWriteEnd);
                        SetState(kS_SongCacheCreateMissing);
                        break;
                    default:
                        UpdateStatus(kStatus_PhysicalWriteEnd);
                        MILO_FAIL(
                            "SaveLoadManager - kS_SongCacheCreateMountWrite unhandled error %d",
                            (int)res
                        );
                        SetState(kS_SongCacheCreateError);
                        break;
                    }
                }
                break;
            case kS_SongCacheCreateFileSize:
                if (mCache->IsDone()) {
                    CacheResult res = mCache->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        SetState(kS_SongCacheCreateRead);
                        break;
                    case kCache_ErrorCacheNotFound:
                        SetState(kS_SongCacheCreateWrite);
                        break;
                    case kCache_ErrorStorageDeviceMissing:
                        SetState(kS_SongCacheCreateMissing);
                        break;
                    default:
                        SetState(kS_SongCacheCreateError);
                        break;
                    }
                }
                break;
            case kS_SongCacheCreateMountDelete:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus(kStatus_PhysicalWriteEnd);
                    SetState(kS_SongCacheCreateMountWrite);
                }
                break;
            case kS_SongCacheCreateRead:
                if (mCache->IsDone()) {
                    CacheResult res = mCache->GetLastResult();
                    switch (res) {
                    case kCache_NoError: {
                        BufStream stream(mData, mSongCacheSize, true);
                        TheSongMgr.LoadCachedSongInfo(stream);
                        SetState(kS_SongCacheCreateUnmount);
                        break;
                    }
                    default:
                        SetState(kS_SongCacheCreateError);
                        break;
                    }
                }
                break;
            case kS_SongCacheCreateWrite:
            case kS_GlobalCreateWrite:
            case kS_GlobalOptionsAutoWrite:
                if (mCache->IsDone()) {
                    mLastCacheResult = mCache->GetLastResult();
                    switch (mState) {
                    case kS_SongCacheCreateWrite:
                        SetState(kS_SongCacheCreateUnmountWrite);
                        break;
                    case kS_GlobalCreateWrite:
                        SetState(kS_GlobalCreateUnmountWrite);
                        break;
                    case kS_GlobalOptionsAutoWrite:
                        SetState(kS_GlobalOptionsAutoUnmountWrite);
                        break;
                    default:
                        MILO_FAIL("Impossible state.");
                        break;
                    }
                }
                break;

            case kS_SongCacheCreateUnmount:
                if (TheCacheMgr->IsDone()) {
                    if (TheCacheMgr->GetLastResult() == kCache_NoError) {
                        SetState(kS_SongCacheCreateDone);
                    } else {
                        SetState(kS_SongCacheCreateError);
                    }
                }
                break;
            case kS_SongCacheCreateUnmountWrite:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus(kStatus_PhysicalWriteEnd);
                    if (mLastCacheResult == kCache_NoError) {
                        mLastCacheResult = TheCacheMgr->GetLastResult();
                    }
                    if (mLastCacheResult == kCache_NoError) {
                        SetState(kS_SongCacheCreateDone);
                    } else {
                        SetState(kS_SongCacheCreateError);
                    }
                }
                break;

            case kS_GlobalCreateSearch:
                if (TheCacheMgr->IsDone()) {
                    mLastCacheResult = TheCacheMgr->GetLastResult();
                    switch (mLastCacheResult) {
                    case kCache_NoError:
                        TheCacheMgr->AddCacheID(mCacheID, kStrGlobalCacheName);
                        SetState(kS_GlobalCreateMountRead);
                        break;
                    case kCache_ErrorCacheNotFound:
                        switch (mDeviceIDState) {
                        case kDeviceID_Invalid:
                            SetState(kS_GlobalCreateChoose);
                            break;
                        case kDeviceID_Chosen:
                            SetState(kS_GlobalCreateChooseAuto);
                            break;
                        default:
                            SetState(kS_GlobalCreateNotFound_Msg);
                            break;
                        }
                        break;
                    default:
                        MILO_FAIL(
                            "SaveLoadManager - CacheMgr search returned error %d",
                            (int)mLastCacheResult
                        );
                        SetState(kS_GlobalCreateError);
                        break;
                    }
                }
                break;

            case kS_GlobalCreateChoose:
            case kS_GlobalCreateChooseAuto:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        mDeviceIDState = kDeviceID_Chosen;
                        mLastChosenDeviceID = mCacheID->GetDeviceID();
                        TheCacheMgr->AddCacheID(mCacheID, kStrGlobalCacheName);
                        SetState(kS_GlobalCreateMountWrite);
                        break;
                    case kCache_ErrorUserCancel:
                        mDeviceIDState = kDeviceID_Declined;
                        SetState(kS_GlobalCreateNotFound_Msg);
                        break;
                    default:
                        MILO_FAIL(
                            "SaveLoadManager - CacheMgr choose returned error %d",
                            (int)res
                        );
                        SetState(kS_GlobalCreateError);
                        break;
                    }
                }
                break;
            case kS_GlobalCreateChooseWaitOnGuide:
                if (!ThePlatformMgr.GuideShowing()) {
                    SetState(kS_GlobalCreateChoose);
                }
                break;
            case kS_GlobalCreateMountRead:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        SetState(kS_GlobalCreateRead);
                        break;
                    case kCache_ErrorCorrupt:
                        SetState(kS_GlobalCreateCorrupt);
                        break;
                    case kCache_ErrorStorageDeviceMissing:
                        SetState(kS_GlobalCreateMissing);
                        break;
                    default:
                        MILO_NOTIFY(
                            "SaveLoadManager - unknown error %d during state %d.",
                            (int)res,
                            (int)mState
                        );
                        SetState(kS_GlobalCreateError);
                        break;
                    }
                }
                break;
            case kS_GlobalCreateMountWrite:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        SetState(kS_GlobalCreateWrite);
                        break;
                    case kCache_ErrorCorrupt:
                        UpdateStatus(kStatus_PhysicalWriteEnd);
                        SetState(kS_GlobalCreateCorrupt);
                        break;
                    case kCache_ErrorStorageDeviceMissing:
                        UpdateStatus(kStatus_PhysicalWriteEnd);
                        SetState(kS_GlobalCreateMissing);
                        break;
                    default:
                        UpdateStatus(kStatus_PhysicalWriteEnd);
                        MILO_NOTIFY(
                            "SaveLoadManager - unknown error %d during state %d.",
                            (int)res,
                            (int)mState
                        );
                        SetState(kS_GlobalCreateError);
                        break;
                    }
                }
                break;
            case kS_GlobalCreateMountDelete:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus(kStatus_PhysicalWriteEnd);
                    SetState(kS_GlobalCreateMountWrite);
                }
                break;
            case kS_GlobalCreateRead:
                if (mCache->IsDone()) {
                    if (mCache->GetLastResult() == kCache_NoError) {
                        int size = TheProfileMgr.GetGlobalOptionsSize();
                        FixedSizeSaveableStream stream(mData, size, true);
                        TheProfileMgr.LoadGlobalOptions(stream);
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)1);
                    } else {
                        TheProfileMgr.SetGlobalOptionsSaveState((ProfileSaveState)2);
                    }
                    SetState(kS_GlobalCreateUnmount);
                }
                break;
            case kS_GlobalOptionsChoose:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        mDeviceIDState = kDeviceID_Chosen;
                        mLastChosenDeviceID = mCacheID->GetDeviceID();
                        TheCacheMgr->AddCacheID(mCacheID, kStrGlobalCacheName);
                        SetState(kS_GlobalOptionsAutoMount);
                        break;
                    case kCache_ErrorUserCancel:
                        mDeviceIDState = kDeviceID_Declined;
                        SetState(kS_GlobalOptionsMissing_Msg);
                        break;
                    default:
                        MILO_NOTIFY(
                            "SaveLoadManager - CacheMgr choose returned error %d",
                            (int)res
                        );
                        SetState(kS_GlobalOptionsError);
                        break;
                    }
                }
                break;
            case kS_GlobalOptionsChooseWaitOnGuide:
                if (!ThePlatformMgr.GuideShowing()) {
                    SetState(kS_GlobalOptionsChoose);
                }
                break;
            case kS_GlobalOptionsAutoMount:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    switch (res) {
                    case kCache_NoError:
                        SetState(kS_GlobalOptionsAutoWrite);
                        break;
                    case kCache_ErrorStorageDeviceMissing:
                        UpdateStatus(kStatus_PhysicalWriteEnd);
                        SetState(kS_GlobalOptionsMissing_Msg);
                        break;
                    default:
                        UpdateStatus(kStatus_PhysicalWriteEnd);
                        MILO_FAIL(
                            "SaveLoadManager - CacheMgr choose returned error %d",
                            (int)res
                        );
                        SetState(kS_GlobalOptionsError);
                        break;
                    }
                }
                break;

            case kS_GlobalCreateUnmount:
                if (TheCacheMgr->IsDone()) {
                    CacheResult res = TheCacheMgr->GetLastResult();
                    if (res == kCache_NoError) {
                        TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileLoaded);
                        SetState(kS_GlobalCreateDone);
                    } else {
                        TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileError);
                        SetState(kS_GlobalCreateDone);
                    }
                }
                break;
            case kS_GlobalCreateUnmountWrite:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus(kStatus_PhysicalWriteEnd);
                    if (mLastCacheResult == kCache_NoError) {
                        mLastCacheResult = TheCacheMgr->GetLastResult();
                    }
                    if (mLastCacheResult == kCache_NoError) {
                        TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileLoaded);
                        SetState(kS_GlobalCreateDone);
                    } else {
                        TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileError);
                        SetState(kS_GlobalCreateDone);
                    }
                }
                break;
            case kS_GlobalOptionsAutoUnmountWrite:
                if (TheCacheMgr->IsDone()) {
                    UpdateStatus(kStatus_PhysicalWriteEnd);
                    if (mLastCacheResult == kCache_NoError) {
                        mLastCacheResult = TheCacheMgr->GetLastResult();
                    }
                    if (mLastCacheResult == kCache_NoError) {
                        TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileLoaded);
                        SetState(kS_GlobalOptionsDone);
                    } else {
                        TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileError);
                        SetState(kS_GlobalOptionsDone);
                    }
                }
                break;

            case kS_SaveOverwrite:
            case kS_SaveNoOverwrite:
                if (!mWaiting) {
                    UpdateStatus(kStatus_PhysicalWriteEnd);
                    switch (mLastMCResult) {
                    case kMCNoError:
                        SetState(kS_SaveLoadSuccess);
                        break;
                    case kMCNoCard:
                        SetState(kS_SaveDeviceInvalid);
                        break;
                    case kMCNotEnoughSpace:
                        mDeviceIDState = kDeviceID_Invalid;
                        mLastChosenDeviceID = 0;
                        SetState(kS_SaveNotEnoughSpace);
                        break;
                    case kMCFileExists:
                        MILO_ASSERT(mState != kS_SaveOverwrite, 0x2ED);
                        SetState(kS_SaveConfirmOverwrite);
                        break;
                    default:
                        SetState(kS_SaveFailed);
                        break;
                    }
                }
                break;

            case kS_AutosaveSongCache:
                if (TheSongMgr.IsSongCacheWriteDone()) {
                    if (TheProfileMgr.GlobalOptionsNeedsSave()) {
                        SetState(kS_AutosaveGlobal);
                    } else {
                        SetState(kS_AutosaveFindNext);
                    }
                }
                break;

            case kS_Abort:
            case kS_Finish:
                if (!mWaiting) {
                    if (mCache) {
                        if (mCache->IsDone()) {
                            TheCacheMgr->UnmountAsync(&mCache, nullptr);
                        }
                    } else if (TheCacheMgr->IsDone()) {
                        if (mState == kS_Abort) {
                            SetState(kS_Done);
                        } else {
                            SetState(kS_None);
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
        case kS_None:
            b14 = true;
            break;
        case kS_AutoloadStartLoad:
        case kS_SaveOverwrite:
        case kS_SaveNoOverwrite:
        case kS_ManualLoadStartLoad:
            if (newState != kS_Abort) {
                RELEASE(mAction);
            }
            break;
        case kS_SongCacheCreateRead:
        case kS_SongCacheCreateWrite:
        case kS_GlobalCreateRead:
        case kS_GlobalCreateWrite:
        case kS_GlobalOptionsAutoWrite:
            if (newState != kS_Finish) {
                if (mData) {
                    MemFree(mData, __FILE__, 0x424);
                    mData = nullptr;
                }
            }
            break;
        case kS_Abort:
            RELEASE(mAction);
            break;
        case kS_Finish:
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
            UpdateStatus(kStatus_Start);
        }
        switch (mState) {
        case kS_None:
            UpdateStatus(kStatus_End);
            break;
        case kS_Startup:
            mDeviceIDState = kDeviceID_Invalid;
            break;
        case kS_AutoloadInit:
            if (mFirstTime) {
                SetState(kS_SongCacheCreateSearch);
            } else {
                SetState(kS_AutoloadFindNext);
            }
            break;
        case kS_AutoloadFindNext:
            mProfileCur = GetNewSigninProfile();
            if (!mProfileCur) {
                SetState(kS_AutoloadAllDone);
            } else {
                SetState(kS_AutoloadSearch);
            }
            break;
        case kS_AutoloadSearch: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x48B);
            mWaiting = true;
            TheMemcardMgr.OnSearchForDevice(pProfile);
            break;
        }
        case kS_AutoloadNoSaveFound:
            if (mDeviceIDState == kDeviceID_Chosen) {
                SetState(kS_AutoloadChooseDeviceManual);
            } else {
                SetState(kS_AutoloadNoSaveFound_Msg);
            }
            break;
        case kS_AutoloadChooseDeviceManual:
            MILO_ASSERT(mDeviceIDState == kDeviceID_Chosen, 0x4AB);
            TheMemcardMgr.SetDevice(mLastChosenDeviceID);
            SetState(kS_AutoloadStartLoad);
            break;
        case kS_AutoloadChooseDeviceNone: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x4B6);
            mWaiting = true;
            TheMemcardMgr.SelectDevice(pProfile, this, mLastSelectionPadNum, false);
            break;
        }
        case kS_AutoloadChooseDeviceMulti:
        case kS_AutoloadChooseDeviceMissing:
        case kS_SaveChooseDeviceInvalid: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x4C7);
            mWaiting = true;
            TheMemcardMgr.SelectDevice(pProfile, this, mLastSelectionPadNum, false);
            break;
        }
        case kS_AutoloadStartLoad: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x4D6);
            mWaiting = true;
            RELEASE(mAction);
            mAction = new LoadMemcardAction(pProfile);
            pProfile->PreLoad();
            TheMemcardMgr.OnLoadGame(pProfile, mAction);
            break;
        }
        case kS_AutoloadAllDone:
            mFirstTime = false;
            if (TheProfileMgr.GlobalOptionsNeedsSave()) {
                SetState(kS_AutoloadSaveGlobal);
            } else {
                TheProfileMgr.HandleProfileLoadComplete();
                SetState(kS_Done);
            }
            break;
        case kS_ManualSaveDone:
            SetState(kS_Done);
            break;
        case kS_SongCacheCreateSearch:
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
        case kS_SongCacheCreateNotFound:
        case kS_SongCacheCreateMissing:
            SetState(kS_SongCacheCreateChoose);
            break;
        case kS_SongCacheCreateChoose: {
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
                if (TheCacheMgr->GetLastResult() != kCache_NoError) {
                    SetState(kS_SongCacheCreateChooseWaitOnGuide);
                }
            }
            break;
        }
        case kS_SongCacheCreateMountRead:
        case kS_GlobalCreateMountRead:
            if (!TheCacheMgr->MountAsync(mCacheID, &mCache, nullptr)) {
                MILO_FAIL(
                    "TheCacheMgr->MountAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;

        case kS_SongCacheCreateMountWrite:
        case kS_GlobalCreateMountWrite:
        case kS_GlobalOptionsAutoMount:
            UpdateStatus(kStatus_PhysicalWriteStart);
            if (!TheCacheMgr->MountAsync(mCacheID, &mCache, nullptr)) {
                MILO_FAIL(
                    "TheCacheMgr->MountAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case kS_SongCacheCreateMountDelete:
        case kS_GlobalCreateMountDelete:
            UpdateStatus(kStatus_PhysicalWriteStart);
            if (!TheCacheMgr->DeleteAsync(mCacheID)) {
                MILO_FAIL(
                    "TheCacheMgr->DeleteAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case kS_SongCacheCreateRead:
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
        case kS_SongCacheCreateFileSize:
            if (!mCache->GetFileSizeAsync(
                    mSongCacheName.c_str(), &mSongCacheSize, nullptr
                )) {
                MILO_FAIL(
                    "mCache->GetFileSizeAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case kS_SongCacheCreateWrite: {
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
        case kS_SongCacheCreateUnmount:
        case kS_SongCacheCreateUnmountWrite:
            if (!TheCacheMgr->UnmountAsync(&mCache, nullptr)) {
                MILO_FAIL(
                    "TheCacheMgr->UnmountAsync failed with CacheResult %d",
                    (int)TheCacheMgr->GetLastResult()
                );
            }
            break;
        case kS_SongCacheCreateDecline:
            mDeviceIDState = kDeviceID_Declined;
            mLastChosenDeviceID = 0;
            mSongCacheWriteDisabled = true;
            SetState(mCache ? kS_SongCacheCreateUnmount : kS_SongCacheCreateDone);
            break;
        case kS_SongCacheCreateError:
            mDeviceIDState = kDeviceID_Invalid;
            mLastChosenDeviceID = 0;
            mSongCacheWriteDisabled = true;
            SetState(mCache ? kS_SongCacheCreateUnmount : kS_SongCacheCreateDone);
            break;
        case kS_SongCacheCreateDone:
            mCacheID = nullptr;
            SetState(kS_GlobalCreateSearch);
            break;

        case kS_AutoloadSaveGlobal:
            if (!mCacheID) {
                mCacheID = TheCacheMgr->GetCacheID(kStrGlobalCacheName);
            }
            if (!mCacheID) {
                SetState(kS_GlobalCreateError);
            } else {
                SetState(kS_GlobalCreateMountWrite);
            }
            break;
        case kS_GlobalCreateSearch:
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
        case kS_GlobalCreateMissing:
            if (mDeviceIDState != kDeviceID_Invalid) {
                mDeviceIDState = kDeviceID_Invalid;
                SetState(kS_GlobalCreateMissing_Msg);
            } else {
                SetState(kS_GlobalCreateChoose);
            }
            break;
        case kS_GlobalCreateChoose: {
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
                if (TheCacheMgr->GetLastResult() != kCache_NoError) {
                    SetState(kS_GlobalCreateChooseWaitOnGuide);
                }
            }
            break;
        }
        case kS_GlobalCreateChooseAuto: {
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
        case kS_GlobalOptionsMissing:
            if (mDeviceIDState == kDeviceID_Invalid
                || mDeviceIDState == kDeviceID_Chosen) {
                SetState(kS_GlobalOptionsChoose);
            } else {
                SetState(kS_GlobalOptionsMissing_Msg);
            }
            break;
        case kS_GlobalOptionsChoose: {
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
                if (TheCacheMgr->GetLastResult() != kCache_NoError) {
                    SetState(kS_GlobalOptionsChooseWaitOnGuide);
                }
            }
            break;
        }
        case kS_GlobalCreateRead: {
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
        case kS_GlobalCreateWrite:
        case kS_GlobalOptionsAutoWrite: {
            UpdateStatus(kStatus_PhysicalWriteStart);
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
        case kS_GlobalCreateUnmount:
        case kS_GlobalCreateUnmountWrite:
        case kS_GlobalOptionsAutoUnmountWrite:
            if (!TheCacheMgr->UnmountAsync(&mCache, nullptr)) {
                if (TheCacheMgr->GetLastResult() != kCache_ErrorStorageDeviceMissing) {
                    MILO_NOTIFY(
                        "UnmountAsync failed with error %d",
                        (int)TheCacheMgr->GetLastResult()
                    );
                }
            }
            break;
        case kS_GlobalCreateDecline:
            mDeviceIDState = kDeviceID_Declined;
            mLastChosenDeviceID = 0;
            TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileError);
            SetState(kS_GlobalCreateDone);
            break;
        case kS_GlobalCreateError:
            mDeviceIDState = kDeviceID_Invalid;
            mLastChosenDeviceID = 0;
            TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileError);
            SetState(kS_GlobalCreateDone);
            break;
        case kS_GlobalCreateDone:
            if (TheProfileMgr.GetNewlySignedIn().size() > 1) {
                mDeviceIDState = kDeviceID_Declined;
            }
            SetState(kS_AutoloadFindNext);
            break;
        case kS_GlobalOptionsError:
            mDeviceIDState = kDeviceID_Invalid;
            mLastChosenDeviceID = 0;
            TheProfileMgr.SetGlobalOptionsSaveState(kMetaProfileError);
            SetState(kS_GlobalOptionsDone);
            break;
        case kS_SaveLoadError: {
            mDeviceIDState = kDeviceID_Invalid;
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x6FE);
            TheMemcardMgr.SaveLoadProfileComplete(pProfile, 2);
            TheUIEventMgr->TriggerEvent(saveload_dialog_event, nullptr);
            break;
        }
        case kS_AutoloadNoSaveFound_Msg:
        case kS_AutoloadMultipleSavesFound:
        case kS_AutoloadDeviceMissing:
        case kS_AutoloadCorrupt:
        case kS_AutoloadNotOwner:
        case kS_AutoloadObsolete:
        case kS_AutoloadFuture:
        case kS_SongCacheCreateNotFound_Msg:
        case kS_SongCacheCreateMissing_Msg:
        case kS_SongCacheCreateCorrupt:
        case kS_GlobalCreateNotFound_Msg:
        case kS_GlobalCreateMissing_Msg:
        case kS_GlobalCreateCorrupt:
        case kS_GlobalOptionsMissing_Msg:
        case kS_SaveConfirmOverwrite:
        case kS_SaveNotEnoughSpace:
        case kS_SaveNotEnoughSpacePS3:
        case kS_SaveDeviceInvalid:
        case kS_SaveFailed:
        case kS_SaveDisabledByCheat:
        case kS_LoadFailed:
        case kS_ManualSaveNoDevice:
        case kS_ManualLoadConfirmUnsaved:
        case kS_ManualLoadConfirm:
        case kS_ManualLoadNoDevice:
        case kS_ManualLoadMissing:
        case kS_ManualLoadNoFile:
        case kS_ManualLoadCorrupt:
        case kS_ManualLoadNotOwner:
            TheUIEventMgr->TriggerEvent(saveload_dialog_event, nullptr);
            break;
        case kS_SaveLoadSuccess:
        case kS_SaveLoadUnchanged: {
            mDeviceIDState = kDeviceID_Invalid;
            int i16 = mState != kS_SaveLoadSuccess ? -1 : 1;
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x713);
            TheMemcardMgr.SaveLoadProfileComplete(pProfile, i16);
            switch (mMode) {
            case kAutoLoad:
                SetState(kS_AutoloadFindNext);
                break;
            case kAutoSave:
                SetState(kS_AutosaveFindNext);
                break;
            default:
                break;
            }
            break;
        }
        case kS_GlobalOptionsDone:
            SetState(kS_AutosaveFindNext);
            break;
        case kS_SaveLookForFile: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x72d);
            mWaiting = true;
            TheMemcardMgr.OnCheckForSaveContainer(pProfile);
            break;
        }
        case kS_SaveOverwrite: {
            UpdateStatus(kStatus_PhysicalWriteStart);
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x739);
            mWaiting = true;
            RELEASE(mAction);
            mAction = new SaveMemcardAction(pProfile);
            TheMemcardMgr.OnSaveGame(pProfile, mAction, 1);
            break;
        }
        case kS_SaveNoOverwrite: {
            UpdateStatus(kStatus_PhysicalWriteStart);
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x747);
            mWaiting = true;
            RELEASE(mAction);
            mAction = new SaveMemcardAction(pProfile);
            TheMemcardMgr.OnSaveGame(pProfile, mAction, 0);
            break;
        }
        case kS_SaveDeleteSavesPS3: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x76d);
            mWaiting = true;
            TheMemcardMgr.OnDeleteSaves(pProfile);
            break;
        }
        case kS_AutosaveInit:
            if (SongCacheNeedsWrite()) {
                SetState(kS_AutosaveSongCache);
            } else if (TheProfileMgr.GlobalOptionsNeedsSave()) {
                SetState(kS_AutosaveGlobal);
            } else {
                SetState(kS_AutosaveFindNext);
            }
            break;
        case kS_AutosaveSongCache:
            TheSongMgr.StartSongCacheWrite();
            break;
        case kS_AutosaveGlobal:
            if (!mCacheID) {
                mCacheID = TheCacheMgr->GetCacheID(kStrGlobalCacheName);
            }
            if (!mCacheID) {
                SetState(kS_GlobalOptionsError);
            } else {
                SetState(kS_GlobalOptionsAutoMount);
            }
            break;
        case kS_AutosaveFindNext:
            mProfileCur = GetAutosavableProfile();
            if (mProfileCur) {
                if (TheMemcardMgr.IsStorageDeviceValid(mProfileCur)) {
                    SetState(kS_SaveOverwrite);
                } else {
                    SetState(kS_SaveDeviceInvalid);
                }
            } else {
                SetState(kS_AutosaveAllDone);
            }
            break;
        case kS_AutosaveAllDone:
            TheProfileMgr.HandleProfileSaveComplete();
            SetState(kS_Done);
            break;
        case kS_ManualSaveInit:
            SetState(kS_ManualSaveChooseDevice);
            break;
        case kS_ManualSaveChooseDevice:
        case kS_ManualLoadChooseDevice: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x7d6);
            mWaiting = true;
            TheMemcardMgr.SelectDevice(pProfile, this, mLastSelectionPadNum, true);
            break;
        }
        case kS_ManualLoadInit: {
            int pad = 0;
            if (mProfileCur) {
                pad = mProfileCur->GetPadNum();
            }
            if (TheProfileMgr.HasUnsavedDataForPad(pad)) {
                SetState(kS_ManualLoadConfirmUnsaved);
            } else {
                SetState(kS_ManualLoadConfirm);
            }
            break;
        }
        case kS_ManualLoadStartLoad: {
            HamProfile *pProfile = mProfileCur;
            MILO_ASSERT(pProfile, 0x811);
            mWaiting = true;
            RELEASE(mAction);
            mAction = new LoadMemcardAction(pProfile);
            pProfile->PreLoad();
            TheMemcardMgr.OnLoadGame(pProfile, mAction);
            break;
        }
        case kS_Done:
            TheMemcardMgr.SaveLoadAllComplete();
            Finish();
            break;
        default:
            break;
        }
    }
}
