#pragma once
#include "EventDialogPanel.h"
#include "meta/MemcardAction.h"
#include "meta/MemcardMgr.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/UIEventMgr.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Memcard.h"
#include "os/PlatformMgr.h"
#include "utl/Cache.h"
#include "utl/Str.h"

enum SaveLoadMgrStatus {
    kStatus_Start = 0,
    kStatus_PhysicalWriteStart = 1,
    kStatus_PhysicalWriteEnd = 2,
    kStatus_AutoloadStart = 3,
    kStatus_AutoloadEnd = 4,
    kStatus_End = 5,
};

enum DeviceIDState {
    kDeviceID_Invalid = 0,
    kDeviceID_Declined = 1,
    kDeviceID_Chosen = 2,
};

class SaveLoadManager : public Hmx::Object {
public:
    enum SaveLoadMode {
        kAutoLoad = 0,
        kAutoSave = 1,
        kDisableAutoSave = 2
    };
    enum State {
        kS_None = 0x0000,
        kS_Startup = 0x0001,
        kS_AutoloadInit = 0x0002,
        kS_AutoloadFindNext = 0x0003,
        kS_AutoloadSearch = 0x0004,
        kS_AutoloadNoSaveFound = 0x0005,
        kS_AutoloadNoSaveFound_Msg = 0x0006,
        kS_AutoloadMultipleSavesFound = 0x0007,
        kS_AutoloadChooseDeviceNone = 0x0008,
        kS_AutoloadChooseDeviceManual = 0x0009,
        kS_AutoloadChooseDeviceMulti = 0x000a,
        kS_AutoloadStartLoad = 0x000b,
        kS_AutoloadDeviceMissing = 0x000c,
        kS_AutoloadChooseDeviceMissing = 0x000d,
        kS_AutoloadCorrupt = 0x000e,
        kS_AutoloadNotOwner = 0x000f,
        kS_AutoloadObsolete = 0x0010,
        kS_AutoloadFuture = 0x0011,
        kS_AutoloadAllDone = 0x0012,
        kS_AutoloadSaveGlobal = 0x0013,
        kS_SongCacheCreateSearch = 0x0014,
        kS_SongCacheCreateNotFound = 0x0015,
        kS_SongCacheCreateMissing = 0x0016,
        kS_SongCacheCreateNotFound_Msg = 0x0017,
        kS_SongCacheCreateMissing_Msg = 0x0018,
        kS_SongCacheCreateChoose = 0x0019,
        kS_SongCacheCreateChooseWaitOnGuide = 0x001a,
        kS_SongCacheCreateMountRead = 0x001b,
        kS_SongCacheCreateCorrupt = 0x001c,
        kS_SongCacheCreateMountDelete = 0x001d,
        kS_SongCacheCreateFileSize = 0x001e,
        kS_SongCacheCreateRead = 0x001f,
        kS_SongCacheCreateMountWrite = 0x0020,
        kS_SongCacheCreateWrite = 0x0021,
        kS_SongCacheCreateUnmount = 0x0022,
        kS_SongCacheCreateUnmountWrite = 0x0023,
        kS_SongCacheCreateDecline = 0x0024,
        kS_SongCacheCreateError = 0x0025,
        kS_SongCacheCreateDone = 0x0026,
        kS_GlobalCreateSearch = 0x0027,
        kS_GlobalCreateMissing = 0x0028,
        kS_GlobalCreateNotFound_Msg = 0x0029,
        kS_GlobalCreateMissing_Msg = 0x002a,
        kS_GlobalCreateChoose = 0x002b,
        kS_GlobalCreateChooseAuto = 0x002c,
        kS_GlobalCreateChooseWaitOnGuide = 0x002d,
        kS_GlobalCreateMountRead = 0x002e,
        kS_GlobalCreateCorrupt = 0x002f,
        kS_GlobalCreateMountDelete = 0x0030,
        kS_GlobalCreateMountWrite = 0x0031,
        kS_GlobalCreateRead = 0x0032,
        kS_GlobalCreateWrite = 0x0033,
        kS_GlobalCreateUnmount = 0x0034,
        kS_GlobalCreateUnmountWrite = 0x0035,
        kS_GlobalCreateDecline = 0x0036,
        kS_GlobalCreateError = 0x0037,
        kS_GlobalCreateDone = 0x0038,
        kS_GlobalOptionsMissing = 0x0039,
        kS_GlobalOptionsMissing_Msg = 0x003a,
        kS_GlobalOptionsChoose = 0x003b,
        kS_GlobalOptionsChooseWaitOnGuide = 0x003c,
        kS_GlobalOptionsAutoMount = 0x003d,
        kS_GlobalOptionsAutoWrite = 0x003e,
        kS_GlobalOptionsAutoUnmountWrite = 0x003f,
        kS_GlobalOptionsError = 0x0040,
        kS_GlobalOptionsDone = 0x0041,
        kS_SaveLoadError = 0x0042,
        kS_SaveLoadSuccess = 0x0043,
        kS_SaveLoadUnchanged = 0x0044,
        kS_SaveLookForFile = 0x0045,
        kS_SaveOverwrite = 0x0046,
        kS_SaveNoOverwrite = 0x0047,
        kS_SaveConfirmOverwrite = 0x0048,
        kS_SaveNotEnoughSpace = 0x0049,
        kS_SaveNotEnoughSpacePS3 = 0x004a,
        kS_SaveDeleteSavesPS3 = 0x004b,
        kS_SaveDeviceInvalid = 0x004c,
        kS_SaveChooseDeviceInvalid = 0x004d,
        kS_SaveFailed = 0x004e,
        kS_SaveDisabledByCheat = 0x004f,
        kS_LoadFailed = 0x0050,
        kS_AutosaveInit = 0x0051,
        kS_AutosaveSongCache = 0x0052,
        kS_AutosaveGlobal = 0x0053,
        kS_AutosaveFindNext = 0x0054,
        kS_AutosaveAllDone = 0x0055,
        kS_ManualSaveInit = 0x0056,
        kS_ManualSaveChooseDevice = 0x0057,
        kS_ManualSaveNoDevice = 0x0058,
        kS_ManualSaveDone = 0x0059,
        kS_ManualLoadInit = 0x005a,
        kS_ManualLoadConfirmUnsaved = 0x005b,
        kS_ManualLoadConfirm = 0x005c,
        kS_ManualLoadChooseDevice = 0x005d,
        kS_ManualLoadNoDevice = 0x005e,
        kS_ManualLoadMissing = 0x005f,
        kS_ManualLoadStartLoad = 0x0060,
        kS_ManualLoadNoFile = 0x0061,
        kS_ManualLoadCorrupt = 0x0062,
        kS_ManualLoadNotOwner = 0x0063,
        kS_ManualLoadDone = 0x0064,
        kS_Abort = 0x0065,
        kS_Done = 0x0066,
        kS_Finish = 0x0067,
    };
    SaveLoadManager();
    virtual ~SaveLoadManager();
    virtual DataNode Handle(DataArray *, bool);

    bool IsReasonToAutosave();
    bool IsReasonToAutoload();
    void ManualSave(HamProfile *pProfile);
    bool IsAutosaveEnabled(HamProfile *pProfile);
    void EnableAutosave(HamProfile *pProfile);
    void DisableAutosave(HamProfile *pProfile);
    void HandleEventResponse(HamProfile *pProfile, int iChoiceIndex);
    void Activate();
    void PrintoutSaveSizeInfo();
    Symbol GetDialogOpt1();
    Symbol GetDialogOpt2();
    void AutoSave();
    void AutoLoad();
    void HandleEventResponseStart(int);
    bool IsIdle() const;
    int GetDialogFocusOption();
    void Poll();
    bool IsInitialLoadDone() const { return !mFirstTime; }

    DataNode GetDialogMsg();

    static void Init();

private:
    bool SongCacheNeedsWrite();

protected:
    DataNode OnMsg(const DeviceChosenMsg &);
    DataNode OnMsg(const NoDeviceChosenMsg &);
    DataNode OnMsg(const MCResultMsg &);
    DataNode OnMsg(const SigninChangedMsg &);
    DataNode OnMsg(const EventDialogDismissMsg &);

    HamProfile *GetAutosavableProfile();
    HamProfile *GetNewSigninProfile();
    void SetState(State);
    void UpdateStatus(SaveLoadMgrStatus status);
    void Start();
    void Finish();
    bool IsSafePlaceToSave() const;
    bool IsSafePlaceToLoad() const;

    bool mActivated; // 0x2c
    bool mFirstTime; // 0x2d
    SaveLoadMode mMode; // 0x30
    State mState; // 0x34
    State mStateAtSelectStart; // 0x38
    int mLastSelectionPadNum; // 0x3c
    HamProfile *mProfileCur; // 0x40
    String mSongCacheName; // 0x44
    unsigned int mSongCacheSize; // 0x4c
    bool mHasAutoloaded; // 0x50
    CacheID *mCacheID; // 0x54
    Cache *mCache; // 0x58
    void *mData; // 0x5c
    bool mSongCacheWriteDisabled; // 0x60
    bool mWaiting; // 0x61
    MCResult mLastMCResult; // 0x64
    CacheResult mLastCacheResult; // 0x68
    bool mNeedsSave; // 0x6c
    bool mNeedsLoad; // 0x6d
    unsigned int mLastChosenDeviceID; // 0x70
    DeviceIDState mDeviceIDState; // 0x74
    MemcardAction *mAction; // 0x78
};

extern SaveLoadManager *TheSaveLoadMgr;

DECLARE_MESSAGE(SaveLoadMgrStatusUpdateMsg, "saveloadmgr_status_update_msg")
SaveLoadMgrStatusUpdateMsg(int status) : Message(Type(), status) {}
SaveLoadMgrStatus GetStatus() const { return (SaveLoadMgrStatus)mData->Int(2); }
END_MESSAGE
