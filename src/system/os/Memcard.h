#pragma once
#include "obj/Msg.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "xdk/XAPILIB.h"

enum AccessType {
    kAccessRead = 0,
    kAccessWrite = 1
};
enum CreateType {
    // 0 - open existing
    // 1 - open always
    // 2 - create always
};
enum SeekType {
    kSeekBegin,
    kSeekCur,
    kSeekEnd
};
enum MCResult {
    kMCNoError = 0,
    kMCNoCard = 1,
    kMCNotFormatted = 2,
    kMCDifferentCard = 3,
    kMCReadWriteFailed = 4,
    kMCCorrupt = 5,
    kMCNotEnoughSpace = 6,
    kMCFileExists = 7,
    kMCFileNotFound = 8,
    kMCMultipleFilesFound = 9,
    kMCObsoleteVersion = 10,
    kMCNewerVersion = 11,
    kMCGeneralError = 12,
    kMCUnsupported = 13,
    kMCAlreadyFormatted = 14,
    kMCInsufficientInodes = 15,
    kMCSystemCorrupt = 16,
    kMCAccessError = 17,
    kMCMaxedSysMem = 18,
    kMCSystemMemCorrupt = 19,
    kMCUnknownError = 20,
    kMCNoEntriesError = 21,
    kMCNoFilesError = 22,
    kMCNoPermission = 23,
    kMCDeprecated = 24,
    kMCNotOwner = 25,
    kMCMax = 26
};

struct ContainerId {
    void Set(int userIdx, DWORD);

    int mUserIndex; // 0x0
    XCONTENTDEVICEID mDeviceId; // 0x4
    DWORD unk8; // 0x8 - size?
};

class MCFile {
public:
    MCFile() {}
    virtual ~MCFile() {}
    virtual MCResult Open(const char *, AccessType, CreateType) = 0;
    virtual MCResult Read(void *, int) = 0;
    virtual MCResult Write(const void *, int) = 0;
    virtual MCResult Seek(int, SeekType) = 0;
    virtual MCResult Close() = 0;
    virtual bool IsOpen() = 0;

    MEM_OVERLOAD(MCFile, 0x13C);
};

class MCContainer {
public:
    MCContainer(const ContainerId &c) : mContainerID(c), mMounted(false) {}
    virtual ~MCContainer() {}
    virtual MCResult Mount(CreateType) = 0;
    virtual MCResult Unmount() = 0;
    virtual MCResult GetPathFreeSpace(const char *, u64 *) = 0;
    virtual MCResult GetDeviceFreeSpace(u64 *) = 0;
    virtual MCResult Delete(const char *) = 0;
    virtual MCResult RemoveDir(const char *) = 0;
    virtual MCResult MakeDir(const char *) = 0;
    virtual MCResult GetSize(const char *, int *) = 0;
    virtual MCResult Format() = 0;
    virtual MCResult Unformat() = 0;
    virtual MCFile *CreateMCFile() = 0;
    virtual void DestroyMCFile(MCFile *);
    virtual String BuildPath(const char *cc) { return cc; }
    virtual MCResult PrintDir(const char *, bool) = 0;

    MEM_OVERLOAD(MCContainer, 0xFE);
    bool IsMounted() const { return mMounted; }
    void SetMounted(bool mount) { mMounted = mount; }
    const ContainerId &Cid() const { return mContainerID; }

private:
    ContainerId mContainerID; // 0x4
    bool mMounted; // 0x10
};

class Memcard : public Hmx::Object {
public:
    Memcard() {}
    virtual ~Memcard() {}
    virtual void Init();
    virtual void Terminate();
    virtual void Poll();
    virtual void SetContainerName(const char *) {}
    virtual void SetContainerDisplayName(const wchar_t *) {}
    virtual const char *GetContainerName() { return ""; }
    virtual const wchar_t *GetDisplayName() { return L""; }
    virtual void ShowDeviceSelector(const ContainerId &, Hmx::Object *, int, bool);
    virtual bool IsDeviceValid(const ContainerId &) { return true; }
    virtual MCResult DeleteContainer(const ContainerId &) = 0;
    virtual MCContainer *CreateContainer(const ContainerId &) = 0;
    virtual void DestroyContainer(MCContainer *);
};

#include "obj/Msg.h"
DECLARE_MESSAGE(DeviceChosenMsg, "device_chosen")
DeviceChosenMsg(int x) : Message(Type(), x) {}
int Device() const { return mData->Int(2); }
END_MESSAGE

DECLARE_MESSAGE(NoDeviceChosenMsg, "no_device_chosen")
NoDeviceChosenMsg() : Message(Type()) {}
END_MESSAGE
