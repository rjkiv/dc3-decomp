#pragma once
#include "obj/Msg.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "xdk/XAPILIB.h"

enum AccessType {
    kOsModeRead = 0,
    kOsModeWrite = 1
};
enum CreateType {
    kOsOpenExisting = 0,
    kOsOpenAlways = 1,
    kOsCreateNew = 2,
};
enum SeekType {
    kOsSeekBegin = 0,
    kOsSeekCur = 1,
    kOsSeekEnd = 2
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
    DWORD mDeviceId; // 0x4
    DWORD mSize; // 0x8
};

class MCFile;

class MCContainer {
public:
    MCContainer(const ContainerId &c) : mCid(c), mIsMounted(false) {}
    virtual ~MCContainer() {}
    virtual MCResult Mount(CreateType createType) = 0;
    virtual MCResult Unmount() = 0;
    virtual MCResult GetPathFreeSpace(const char *strPath, u64 *pFreeSpace) = 0;
    virtual MCResult GetDeviceFreeSpace(u64 *pFreeSpace) = 0;
    virtual MCResult Delete(const char *file) = 0;
    virtual MCResult RemoveDir(const char *path) = 0;
    virtual MCResult MakeDir(const char *path) = 0;
    virtual MCResult GetSize(const char *file, int *pFileSize) = 0;
    virtual MCResult Format() = 0;
    virtual MCResult Unformat() = 0;
    virtual MCFile *CreateMCFile() = 0;
    virtual void DestroyMCFile(MCFile *pFile);
    virtual String BuildPath(const char *path) { return path; }
    virtual MCResult PrintDir(const char *path, bool recurse) = 0;

    MEM_OVERLOAD(MCContainer, 0xFE);
    bool IsMounted() const { return mIsMounted; }
    void SetMounted(bool mount) { mIsMounted = mount; }
    const ContainerId &Cid() const { return mCid; }

private:
    ContainerId mCid; // 0x4
    bool mIsMounted; // 0x10
};

class MCFile {
public:
    MCFile(MCContainer *c) : mContainer(c) {}
    virtual ~MCFile() {}
    virtual MCResult Open(const char *file, AccessType mode, CreateType createType) = 0;
    virtual MCResult Read(void *data, int size) = 0;
    virtual MCResult Write(const void *data, int size) = 0;
    virtual MCResult Seek(int offset, SeekType seek) = 0;
    virtual MCResult Close() = 0;
    virtual bool IsOpen() = 0;

    MEM_OVERLOAD(MCFile, 0x13C);

protected:
    MCContainer *mContainer; // 0x4
};

class Memcard : public Hmx::Object {
public:
    Memcard() {}
    virtual ~Memcard() {}
    virtual void Init();
    virtual void Terminate();
    virtual void Poll();
    virtual void SetContainerName(const char *containerName) {}
    virtual void SetContainerDisplayName(const wchar_t *displayName) {}
    virtual const char *GetContainerName() { return ""; }
    virtual const wchar_t *GetDisplayName() { return L""; }
    virtual void ShowDeviceSelector(
        const ContainerId &cid, Hmx::Object *pCallbackObj, int padOverride, bool forceShow
    );
    virtual bool IsDeviceValid(const ContainerId &cid) { return true; }
    virtual MCResult DeleteContainer(const ContainerId &cid) = 0;
    virtual MCContainer *CreateContainer(const ContainerId &cid) = 0;
    virtual void DestroyContainer(MCContainer *pContainer);
};

#include "obj/Msg.h"
DECLARE_MESSAGE(DeviceChosenMsg, "device_chosen")
DeviceChosenMsg(int device) : Message(Type(), device) {}
int Device() const { return mData->Int(2); }
END_MESSAGE

DECLARE_MESSAGE(NoDeviceChosenMsg, "no_device_chosen")
NoDeviceChosenMsg() : Message(Type()) {}
END_MESSAGE
