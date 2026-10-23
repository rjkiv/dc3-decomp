#pragma once
#include "os/Memcard.h"
#include "xdk/win_types.h"

class MCContainerXbox : public MCContainer {
public:
    MCContainerXbox(const ContainerId &);
    virtual ~MCContainerXbox() {}
    virtual MCResult Mount(CreateType);
    virtual MCResult Unmount();
    virtual MCResult GetPathFreeSpace(const char *, u64 *);
    virtual MCResult GetDeviceFreeSpace(u64 *);
    virtual MCResult Delete(const char *);
    virtual MCResult RemoveDir(const char *);
    virtual MCResult MakeDir(const char *);
    virtual MCResult GetSize(const char *, int *);
    virtual MCResult Format() { return (MCResult)0xD; }
    virtual MCResult Unformat() { return (MCResult)0xD; }
    virtual MCFile *CreateMCFile();
    virtual String BuildPath(const char *);
    virtual MCResult PrintDir(const char *, bool);

protected:
    String mDriveName; // 0x14
};

class MCFileXbox : public MCFile {
public:
    MCFileXbox(MCContainerXbox *mcx) : MCFile(mcx), mFile(INVALID_HANDLE_VALUE) {}
    virtual ~MCFileXbox() {}
    virtual MCResult Open(const char *, AccessType, CreateType);
    virtual MCResult Read(void *, int);
    virtual MCResult Write(const void *, int);
    virtual MCResult Seek(int, SeekType);
    virtual MCResult Close();
    virtual bool IsOpen();

    MCResult GetSize(int *pFileSize);

protected:
    HANDLE mFile; // 0x8
};

class MemcardXbox : public Memcard {
public:
    MemcardXbox() : mWaitingForDevice(0), mShowDeviceCallbackObj(0), mDevice(0) {}
    virtual ~MemcardXbox() {}
    virtual void Init() { Memcard::Init(); }
    virtual void Terminate() { Memcard::Terminate(); }
    virtual void Poll();
    virtual void SetContainerName(const char *);
    virtual void SetContainerDisplayName(const wchar_t *);
    virtual const char *GetContainerName() { return mFileName; }
    virtual const wchar_t *GetDisplayName() { return mDisplayName; }
    virtual void ShowDeviceSelector(const ContainerId &, Hmx::Object *, int, bool);
    virtual bool IsDeviceValid(const ContainerId &);
    virtual MCResult DeleteContainer(const ContainerId &);
    virtual MCContainer *CreateContainer(const ContainerId &);

    String GenerateDriveName(DWORD deviceId, int padNum);
    MCResult FindValidUnit(ContainerId *pCid);
    char *FileName() { return mFileName; }
    wchar_t *DisplayName() { return mDisplayName; }

protected:
    char mFileName[XCONTENT_MAX_FILENAME_LENGTH]; // 0x2c
    wchar_t mDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH]; // 0x56
    bool mWaitingForDevice; // 0x156
    Hmx::Object *mShowDeviceCallbackObj; // 0x158
    DWORD mDevice; // 0x15c
    XOVERLAPPED mXOverlapped; // 0x160
};

extern MemcardXbox TheMC;
