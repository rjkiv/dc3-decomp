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
        // there's a lot that's supposed to go in here
        kS_AutoloadNoSaveFound_Msg = 6,
        kS_AutoloadMultipleSavesFound = 7,
        kS_AutoloadStartLoad = 0xB,
        kS_AutoloadDeviceMissing = 0xC,
        kS_AutoloadCorrupt = 0xE,
        kS_AutoloadNotOwner = 0xF,
        kS_AutoloadObsolete = 0x10,
        kS_AutoloadFuture = 0x11,
        kS_SongCacheCreateNotFound_Msg = 0x17,
        kS_SongCacheCreateMissing_Msg = 0x18,
        kS_SongCacheCreateCorrupt = 0x1C,
        kS_GlobalCreateNotFound_Msg = 0x29,
        kS_GlobalCreateMissing_Msg = 0x2A,
        kS_GlobalCreateCorrupt = 0x2F,
        kS_GlobalOptionsMissing_Msg = 0x3A,
        kS_SaveLoadError = 0x42,
        kS_SaveLookForFile = 0x45,
        kS_SaveOverwrite = 0x46,
        kS_SaveNoOverwrite = 0x47,
        kS_SaveConfirmOverwrite = 0x48,
        kS_SaveNotEnoughSpace = 0x49,
        kS_SaveNotEnoughSpacePS3 = 0x4A,
        // 0x4B
        kS_SaveDeviceInvalid = 0x4C,
        kS_SaveChooseDeviceInvalid = 0x4D,
        kS_SaveFailed = 0x4E,
        kS_SaveDisabledByCheat = 0x4F,
        kS_LoadFailed = 0x50,
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
    bool IsInitialLoadDone() const { return !unk2d; }

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
END_MESSAGE
