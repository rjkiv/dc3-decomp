#pragma once
#include "Friend.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/OnlineID.h"
#include "os/User.h"
#include "stl/_vector.h"
#include "utl/JobMgr.h"
#include "xdk/XSOCIAL.h"

enum DiskError {
    kNoDiskError,
    kDiskError,
    kWrongDisk,
    kFailedChecksum
};

enum PlatformRegion {
    kRegionNone,
    kRegionNA,
    kRegionEurope,
    kRegionJapan,
    kNumRegions
};

enum NotifyLocation {
    kNotify0,
    kNotify1,
    kNotify2
};

enum QuitType {
    kQuitNone,
    kQuitShutdown,
    kQuitRestart,
    kQuitMenu,
    kQuitDataManager
};

enum ShowGamercardResult {
    kGamercardResultPrivelegeError = -1
};

typedef bool XCallbackFunc(unsigned long &);

class PlatformMgr : public Hmx::Object {
private:
    bool mHasXSocialPhotoPost; // 0x2c
    bool mHasXSocialLinkPost; // 0x2d
    XOVERLAPPED mOverlapped; // 0x30
    int unk4c; // 0x4c - ptr to something
    int mSigninMask; // 0x50
    int mSigninChangeMask; // 0x54
    bool mGuideShowing; // 0x58
    bool mConfirmCancelSwapped; // 0x59
    bool mConnected; // 0x5a
    bool mScreenSaver; // 0x5b
    PlatformRegion mRegion; // 0x5c
    DiskError mDiskError; // 0x60
    JobMgr *mJobMgr; // 0x64

    DataNode OnSignInUsers(DataArray *);

public:
    // Hmx::Object
    virtual ~PlatformMgr();
    virtual DataNode Handle(DataArray *, bool);

    static XCallbackFunc *sXShowCallback;

    PlatformMgr();
    PlatformRegion GetRegion() const;
    bool IsAnyUserSignedIntoLive() const;
    bool IsSignedIntoLive(int) const;
    bool IsSignedIn(int) const;
    bool IsUserSignedIn(const LocalUser *) const;
    bool IsPadNumSignedIn(int) const;
    bool HasPadNumsSigninChanged(int) const;
    bool HasUserSigninChanged(const LocalUser *) const;
    bool IsUserSignedIntoLive(const LocalUser *) const;
    bool HasOnlinePrivilege(int) const;
    bool UserHasOnlinePrivilege(const LocalUser *) const;
    bool IsUserAGuest(const LocalUser *) const;
    bool IsPadAGuest(int) const;
    void ShowUserFriendsUI(const LocalUser *);
    void ShowFriendsUI(int);
    void ShowOfferUI(const LocalUser *);
    void ShowOfferUI(int);
    bool ShowUserPartyUI(const LocalUser *);
    bool ShowPartyUI(int);
    void InviteUserParty(const LocalUser *);
    void InviteParty(int);
    LocalUser *GetOwnerUserOfGuestUser(LocalUser *);
    int GetOwnerOfGuest(int);
    void SetRegion(PlatformRegion);
    void SetDiskError(DiskError);
    void DebugFakeSigninChangeMsg(int);
    bool IsEthernetCableConnected();
    const char *GetName(int) const;
    bool HasCreatedContentPrivilege() const;
    bool HasKinectSharePrvilege() const;
    void ShowControllerRequiredUI(Hmx::Object *);
    bool IsInParty();
    bool IsInPartyWithOthers();
    bool ShowFitnessBodyProfileUI(int);
    void SetBackgroundDownloadPriority(bool);
    void DisableXMP();
    void EnableXMP();
    void SetScreenSaver(bool);
    void CheckMailbox();
    void RunNetStartUtility();
    void SetNotifyUILocation(NotifyLocation);
    bool PollXSocialCapabilities();
    bool QueryXSocialCapabilities();
    void SmartGlassSend(unsigned long, const DataArray *);
    bool IsSmartGlassConnected();
    void UpdateSigninState();
    void SetPadContext(int, int, int) const;
    void SetPadPresence(int, int) const;
    void SetPadProperty(int, int, unsigned short const *) const;
    void EnumerateFriends(int, std::vector<Friend *> &, Hmx::Object *);
    ShowGamercardResult ShowGamercardForPadNum(int, OnlineID const *);

    bool GuideShowing() { return mGuideShowing; }
    bool IsConnected() { return mConnected; }
    bool ScreenSaver() { return mScreenSaver; }
    int SignInMask() const { return mSigninMask; }
    void QueueEnumJob(Job *);
    void CancelEnumJob(int);
    void Init();
    void RegionInit();
    void PreInit();
    DWORD
    ShowDeviceSelectorUI(DWORD, DWORD, DWORD, ULARGE_INTEGER, DWORD *, XOVERLAPPED *);
    bool GetServiceID(const String &, unsigned int &);
    void SignInUsers(int, unsigned long);
};

extern PlatformMgr ThePlatformMgr;

Symbol PlatformRegionToSymbol(PlatformRegion);
PlatformRegion SymbolToPlatformRegion(Symbol);

// arg here is a bool
DECLARE_MESSAGE(DiskErrorMsg, "disk_error")
DiskErrorMsg() : Message(Type(), 0) {}
END_MESSAGE

DECLARE_MESSAGE(SigninChangedMsg, "signin_changed")
SigninChangedMsg(unsigned long u1, unsigned long u2) : Message(Type(), u1, u2) {}
int GetMask() const { return mData->Int(2); }
int GetChangedMask() const { return mData->Int(3); }
END_MESSAGE

DECLARE_MESSAGE(StorageChangedMsg, "storage_changed")
END_MESSAGE
