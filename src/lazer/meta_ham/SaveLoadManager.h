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

enum SaveLoadMode {
    kAutoLoad = 0,
    kAutoSave = 1,
    kDisableAutoSave = 2,
    kManualDelete = 3
};

enum SaveLoadMgrStatus {
};

class SaveLoadManager : public Hmx::Object {
public:
    enum State {
        kS_Idle = 0x0,
        kS_Start = 0x1,
        kS_AutoloadInit = 0x2,
        kS_AutoloadSelectProfile = 0x3,
        kS_AutoloadSearchDevice = 0x4,
        kS_AutoloadDeviceFound = 0x5,
        kS_AutoloadNoSaveFound_Msg = 0x6,
        kS_AutoloadMultipleSavesFound = 0x7,
        kS_AutoloadSetDevice = 0x8,
        kS_AutoloadSelectDevice = 0x9,
        kS_AutoloadSelectDevice2 = 0xA,
        kS_AutoloadStartLoad = 0xB,
        kS_AutoloadDeviceMissing = 0xC,
        kS_AutoloadSelectDevice3 = 0xD,
        kS_AutoloadCorrupt = 0xE,
        kS_AutoloadNotOwner = 0xF,
        kS_AutoloadObsolete = 0x10,
        kS_AutoloadFuture = 0x11,
        kS_AutoloadDone = 0x12,
        kS_SongCacheInit = 0x13,
        kS_SongCacheSearch = 0x14,
        kS_SongCacheSearchResult = 0x15,
        kS_SongCacheCreate = 0x16,
        kS_SongCacheCreateNotFound_Msg = 0x17,
        kS_SongCacheCreateMissing_Msg = 0x18,
        kS_SongCacheMount = 0x19,
        kS_SongCacheMountStart = 0x1A,
        kS_SongCacheRead = 0x1B,
        kS_SongCacheCreateCorrupt = 0x1C,
        kS_SongCacheGetSize = 0x1D,
        kS_SongCacheAllocRead = 0x1E,
        kS_SongCacheWrite = 0x1F,
        kS_SongCacheUnmount = 0x20,
        kS_SongCacheDone = 0x21,
        kS_SongCacheFailed = 0x22,
        kS_SongCacheLookup = 0x23,
        kS_GlobalOptionsInit = 0x24,
        kS_GlobalOptionsSearch = 0x25,
        kS_GlobalOptionsSearchResult = 0x26,
        kS_GlobalOptionsCreate = 0x27,
        kS_GlobalOptionsLookup = 0x28,
        kS_GlobalCreateNotFound_Msg = 0x29,
        kS_GlobalCreateMissing_Msg = 0x2A,
        kS_GlobalMount = 0x2B,
        kS_GlobalMountStart = 0x2C,
        kS_GlobalCreate2 = 0x2D,
        kS_GlobalMount2 = 0x2E,
        kS_GlobalCreateCorrupt = 0x2F,
        kS_GlobalRead = 0x30,
        kS_GlobalDoneRead = 0x31,
        kS_GlobalWrite = 0x32,
        kS_GlobalDoneWrite = 0x33,
        kS_GlobalUnmount = 0x34,
        kS_GlobalDone = 0x35,
        kS_GlobalFailed = 0x36,
        kS_GlobalCacheLookup = 0x37,
        kS_GlobalNewSignIns = 0x38,
        kS_GlobalOptionsSearchResult2 = 0x39,
        kS_GlobalOptionsMissing_Msg = 0x3A,
        kS_GlobalOptionsCreate2 = 0x3B,
        kS_GlobalOptionsRead = 0x3C,
        kS_GlobalOptionsAllocRead = 0x3D,
        kS_GlobalOptionsWrite = 0x3E,
        kS_GlobalOptionsUnmount = 0x3F,
        kS_GlobalOptionsFailed = 0x40,
        kS_GlobalOptionsDone = 0x41,
        kS_SaveLoadError = 0x42,
        kS_SaveLoadError2 = 0x43,
        kS_SaveLoadCheckForFile = 0x44,
        kS_SaveLookForFile = 0x45,
        kS_SaveOverwrite = 0x46,
        kS_SaveNoOverwrite = 0x47,
        kS_SaveConfirmOverwrite = 0x48,
        kS_SaveNotEnoughSpace = 0x49,
        kS_SaveNotEnoughSpacePS3 = 0x4A,
        kS_SaveDeleteSaves = 0x4B,
        kS_SaveDeviceInvalid = 0x4C,
        kS_SaveChooseDeviceInvalid = 0x4D,
        kS_SaveFailed = 0x4E,
        kS_SaveDisabledByCheat = 0x4F,
        kS_LoadFailed = 0x50,
        kS_SaveDone = 0x51,
        kS_SaveSongCache = 0x52,
        kS_SaveGlobalOptions = 0x53,
        kS_SaveCheckProfile = 0x54,
        kS_SaveCheckAutosave = 0x55,
        kS_ManualSaveInit = 0x56,
        kS_ManualSaveChooseDevice = 0x57,
        kS_ManualSaveNoDevice = 0x58,
        kS_ManualSaveDone = 0x59,
        kS_ManualLoadInit = 0x5A,
        kS_ManualLoadConfirmUnsaved = 0x5B,
        kS_ManualLoadConfirm = 0x5C,
        kS_ManualLoadChooseDevice = 0x5D,
        kS_ManualLoadNoDevice = 0x5E,
        kS_ManualLoadMissing = 0x5F,
        kS_ManualLoadStartLoad = 0x60,
        kS_ManualLoadNoFile = 0x61,
        kS_ManualLoadCorrupt = 0x62,
        kS_ManualLoadNotOwner = 0x63,
        kS_ManualLoadDone = 0x64,
        kS_Abort = 0x65,
        kS_Done = 0x66,
        kS_Finish = 0x67,
        kS_Nil = -1
    };
    SaveLoadManager();
    virtual ~SaveLoadManager();
    virtual DataNode Handle(DataArray *, bool);

    bool IsReasonToAutosave();
    bool IsReasonToAutoload();
    void ManualSave(HamProfile *);
    bool IsAutosaveEnabled(HamProfile *);
    void EnableAutosave(HamProfile *);
    void DisableAutosave(HamProfile *);
    void HandleEventResponse(HamProfile *, int);
    void Activate();
    void PrintoutSaveSizeInfo();
    Symbol GetDialogOpt1();
    Symbol GetDialogOpt2();
    void AutoSave();
    void AutoLoad();
    void HandleEventResponseStart(int);
    bool IsIdle() const;
    bool GetDialogFocusOption();
    bool IsInitialLoadDone() const { return !unk2d; }

    DataNode GetDialogMsg();

    static void Init();

    bool GetUnk2c() { return unk2c; }

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
    void UpdateStatus(SaveLoadMgrStatus);
    void Start();
    void Finish();
    bool IsSafePlaceToSave() const;
    bool IsSafePlaceToLoad() const;

    bool unk2c;
    bool unk2d;
    SaveLoadMode mMode; // 0x30
    State mState; // 0x34
    State mStateAtSelectStart; // 0x38
    int unk3c; // 0x3c
    HamProfile *unk40; // 0x40
    String unk44;
    int unk4c;
    bool unk50;
    CacheID *mCacheID; // 0x54
    Cache *mCache; // 0x58
    void *mData; // 0x5c
    bool mSongCacheWriteDisabled; // 0x60
    bool mWaiting; // 0x61
    int unk64;
    CacheResult unk68; // 0x68
    bool mNeedsSave; // 0x6c
    bool mNeedsLoad; // 0x6d
    int mLastChosenDeviceID; // 0x70
    int mDeviceIDState; // 0x74
    MemcardAction *mAction; // 0x78
};

extern SaveLoadManager *TheSaveLoadMgr;

DECLARE_MESSAGE(SaveLoadMgrStatusUpdateMsg, "saveloadmgr_status_update_msg")
SaveLoadMgrStatusUpdateMsg(int status) : Message(Type(), status) {}
int Status() const { return mData->Int(2); }
END_MESSAGE
