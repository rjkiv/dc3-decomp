#pragma once
#include "meta/MemcardAction.h"
#include "meta/Profile.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Memcard.h"
#include "os/PlatformMgr.h"
#include "os/ThreadCall.h"
#include "ui/UI.h"

class MemcardMgr : public Hmx::Object, public ThreadCallback {
public:
    enum State {
        kS_None = 0,
        kS_Search = 1,
        kS_CheckForSaveContainer = 2,
        kS_SaveGame = 3,
        kS_LoadGame = 4,
        kS_DeleteSaves = 5
    };
    MemcardMgr();
    // Hmx::Object
    virtual ~MemcardMgr();
    virtual DataNode Handle(DataArray *, bool);
    // ThreadCallback
    virtual int ThreadStart();
    virtual void ThreadDone(int);

    void SetProfileSaveBuffer(void *, int);
    void SaveLoadProfileComplete(Profile *, int);
    void SaveLoadAllComplete();

    void Init();
    bool IsStorageDeviceValid(Profile *);
    void OnCheckForSaveContainer(Profile *);
    void OnDeleteSaves(Profile *);
    void OnSaveGame(Profile *, MemcardAction *, int);
    void OnLoadGame(Profile *, MemcardAction *);
    void OnSearchForDevice(Profile *);
    void SetDevice(unsigned int);
    void SelectDevice(Profile *, Hmx::Object *, int, bool);

private:
    MCResult ThreadCall_SearchForDevice();
    MCResult ThreadCall_CheckForSaveContainer();
    MCResult ThreadCall_SaveGame();
    MCResult ThreadCall_LoadGame();
    MCResult PerformRead(MCContainer *);
    MCResult PerformWrite(MCContainer *);

protected:
    DataNode OnMsg(const DeviceChosenMsg &);
    DataNode OnMsg(const NoDeviceChosenMsg &);
    DataNode OnMsg(const UIChangedMsg &);
    DataNode OnMsg(const StorageChangedMsg &);
    DataNode OnMsg(const SigninChangedMsg &);

    State mState; // 0x30
    void *unk34; // 0x34 - save data?
    int unk38; // 0x38 - save data len/bytes?
    MemcardAction *mAction; // 0x3c
    int unk40;
    // indexed by padnums
    ContainerId mContainerIDs[4]; // 0x44
    MCContainer *mContainers[4]; // 0x74
    bool mValidDevices[4]; // 0x84
    int unk88;
    bool mSelectDeviceWaiting; // 0x8c
    Hmx::Object *mSelectDeviceCallBackObj; // 0x90
    int mPadNum; // 0x94
    Profile *mProfile; // 0x98
};

extern MemcardMgr TheMemcardMgr;

DECLARE_MESSAGE(SaveLoadAllCompleteMsg, "save_load_all_complete_msg")
SaveLoadAllCompleteMsg() : Message(Type()) {}
END_MESSAGE

DECLARE_MESSAGE(MCResultMsg, "memcard_result")
MCResultMsg(MCResult res) : Message(Type(), res) {}
END_MESSAGE
