#pragma once
#include "meta/ConnectionStatusPanel.h"
#include "os/ContentMgr.h"
#include "os/PlatformMgr.h"
#include "utl/UTF8.h"
#include "xdk/XAPILIB.h"

class XboxContent : public Content {
public:
    XboxContent(const XCONTENT_CROSS_TITLE_DATA &d, int id, int padNum, bool discover);
    virtual ~XboxContent();
    virtual const char *Root() { return mDrive.c_str(); }
    virtual bool OnMemcard() { return Location() == kLocationRemovableMem; }
    virtual ContentLocT Location();
    virtual unsigned long LicenseBits() { return mLicenseBits; }
    virtual bool HasValidLicenseBits() { return mHasValidLicenseBits; }
    virtual bool IsCorrupt() { return mState == 8 && mCorrupt; }
    virtual State GetState() { return mState; }
    virtual void Poll();
    virtual void Mount();
    virtual void Unmount();
    virtual void Delete();
    virtual Symbol FileName() { return mFilename; }
    virtual const char *DisplayName() {
        const unsigned short *displayName =
            reinterpret_cast<const unsigned short *>(mData.szDisplayName);
        return WideCharToChar(displayName);
    }
    virtual unsigned int GetLRM() { return mLRM; }

private:
    XOVERLAPPED *mOverlapped; // 0x4
    XCONTENT_CROSS_TITLE_DATA mData; // 0x8
    unsigned long mLicenseBits; // 0x140
    bool mHasValidLicenseBits; // 0x144
    String mRoot; // 0x148
    String mDrive; // 0x150
    State mState; // 0x158
    int mPadNum; // 0x15c
    bool mDelete; // 0x160
    bool mCorrupt; // 0x161
    Symbol mFilename; // 0x164
    unsigned int mLRM; // 0x168
};

#define kNumberOfBuffers 7
#define kContentRootMaxLength 12

class XboxContentMgr : public ContentMgr {
public:
    // Hmx::Object
    XboxContentMgr() {}
    virtual DataNode Handle(DataArray *, bool);
    // ContentMgr
    virtual void Init();
    virtual void Terminate();
    virtual void StartRefresh();
    virtual void PollRefresh();
    virtual const char *TitleContentPath() { return ContentPath(0); }
    virtual const char *ContentPath(int) { return MakeString("UPDATE:"); }
    virtual bool MountContent(Symbol);
    virtual bool IsMounted(Symbol);
    virtual bool IsCorrupt(Symbol, const char *&);
    virtual bool DeleteContent(Symbol);
    virtual bool IsDeleteDone(Symbol);
    virtual bool GetLicenseBits(Symbol, unsigned long &ul);

private:
    DataNode OnMsg(const SigninChangedMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const StorageChangedMsg &);
    DataNode OnMsg(const ContentInstalledMsg &);

protected:
    virtual void NotifyMounted(Content *);
    virtual void NotifyUnmounted(Content *);
    virtual void NotifyDeleted(Content *);
    virtual void NotifyFailed(Content *);

    bool mPermissionsIncreased; // 0x70
    bool mNeedsPermissions; // 0x71
    HANDLE mContentEnum[kNumberOfBuffers]; // 0x74
    XCONTENT_CROSS_TITLE_DATA mBuffer[kNumberOfBuffers]; // 0x90
    XOVERLAPPED *mOverlapped[kNumberOfBuffers]; // 0x918
    int mId; // 0x934
    int mContentsToDiscover; // 0x938
    bool mEnumerateSaveGameExports; // 0x93c
};

extern XboxContentMgr gContentMgr;
