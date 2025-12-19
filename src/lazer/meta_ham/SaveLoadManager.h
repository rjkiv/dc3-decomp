#pragma once
#include "meta/MemcardMgr.h"
#include "meta_ham/HamMemcardAction.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/UIEventMgr.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Memcard.h"
#include "os/PlatformMgr.h"
#include "utl/Str.h"

enum SaveLoadMgrStatus {
};

class SaveLoadManager : public Hmx::Object {
public:
    enum State {
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
    State unk30; // 0x30
    State unk34; // 0x34
    int unk38;
    int unk3c;
    HamProfile *unk40;
    String unk44;
    int unk4c;
    bool unk50;
    int unk54;
    int unk58;
    void *unk5c;
    bool unk60;
    bool mWaiting; // 0x61
    int unk64;
    int unk68; // 0x68 - CacheResult?
    bool unk6c;
    bool unk6d;
    int unk70; // 0x60 - device id
    int mDeviceIDState; // 0x74
    LoadMemcardAction *unk78; // 0x78
};

extern SaveLoadManager *TheSaveLoadMgr;

DECLARE_MESSAGE(SaveLoadMgrStatusUpdateMsg, "saveloadmgr_status_update_msg")
SaveLoadMgrStatusUpdateMsg(int status) : Message(Type(), status) {}
END_MESSAGE
