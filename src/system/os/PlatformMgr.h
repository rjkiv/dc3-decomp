#pragma once
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/OnlineID.h"
#include "os/Friend.h"
#include "os/User.h"
#include "utl/JobMgr.h"
#include "xdk/XSOCIAL.h"
#include <vector>

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

typedef bool XCallbackFunc(DWORD &);

class PlatformMgr : public Hmx::Object {
public:
    // Hmx::Object
    virtual ~PlatformMgr();
    virtual DataNode Handle(DataArray *, bool);

    PlatformMgr();
    PlatformRegion GetRegion() const;
    bool IsAnyUserSignedIntoLive() const;
    bool IsSignedIntoLive(int padNum) const;
    bool IsSignedIn(int padnum) const;
    bool IsUserSignedIn(const LocalUser *pUser) const;
    bool IsPadNumSignedIn(int padnum) const;
    bool HasPadNumsSigninChanged(int padnum) const;
    bool HasUserSigninChanged(const LocalUser *pUser) const;
    bool IsUserSignedIntoLive(const LocalUser *pUser) const;
    bool HasOnlinePrivilege(int padNum) const;
    bool UserHasOnlinePrivilege(const LocalUser *pUser) const;
    bool IsUserAGuest(const LocalUser *pUser) const;
    bool IsPadAGuest(int padNum) const;
    void ShowUserFriendsUI(const LocalUser *pUser);
    void ShowFriendsUI(int padNum);
    void ShowOfferUI(const LocalUser *pUser);
    void ShowOfferUI(int padNum);
    bool ShowUserPartyUI(const LocalUser *pUser);
    bool ShowPartyUI(int padNum);
    void InviteUserParty(const LocalUser *pUser);
    void InviteParty(int padNum);
    LocalUser *GetOwnerUserOfGuestUser(LocalUser *pUser);
    int GetOwnerOfGuest(int padNum);
    void SetRegion(PlatformRegion region);
    void SetDiskError(DiskError err);
    void DebugFakeSigninChangeMsg(int padnum);
    bool IsEthernetCableConnected();
    const char *GetName(int padnum) const;
    bool HasCreatedContentPrivilege() const;
    bool HasKinectSharePrvilege() const;
    void ShowControllerRequiredUI(Hmx::Object *);
    bool IsInParty();
    bool IsInPartyWithOthers();
    bool ShowFitnessBodyProfileUI(int padNum);
    void SetBackgroundDownloadPriority(bool alwaysAllow);
    void DisableXMP();
    void EnableXMP();
    void SetScreenSaver(bool screensaver);
    void CheckMailbox();
    void RunNetStartUtility();
    void SetNotifyUILocation(NotifyLocation location);
    bool PollXSocialCapabilities();
    bool QueryXSocialCapabilities();
    void SmartGlassSend(DWORD id, const DataArray *dta);
    bool IsSmartGlassConnected();
    void UpdateSigninState();
    void SetPadContext(int padNum, int ctxId, int ctxValue) const;
    void SetPadPresence(int padNum, int ctxValue) const;
    void SetPadProperty(int padNum, int propId, const unsigned short *wstr) const;
    void EnumerateFriends(int, std::vector<Friend *> &, Hmx::Object *);
    ShowGamercardResult ShowGamercardForPadNum(int padNum, const OnlineID *onlineID);
    void Poll();

    bool GuideShowing() { return mGuideShowing; }
    bool IsConnected() { return mConnected; }
    bool ScreenSaver() { return mScreenSaver; }
    int SignInMask() const { return mSigninMask; }
    void QueueEnumJob(Job *job);
    void CancelEnumJob(int jobID);
    void Init();
    void RegionInit();
    void PreInit();
    DWORD
    ShowDeviceSelectorUI(
        DWORD userIndex,
        DWORD contentType,
        DWORD contentFlags,
        ULARGE_INTEGER bytesRequested,
        DWORD *deviceID,
        XOVERLAPPED *overlapped
    );
    bool GetServiceID(const String &, unsigned int &);
    void SignInUsers(int count, DWORD flags);

    static XCallbackFunc *sXShowCallback;
    static Hmx::Object *spShowControllerObject;
    static DWORD sdwShowControllerTrackingID;
    static int snShowControllerPadNum;

private:
    bool mHasXSocialPhotoPost; // 0x2c
    bool mHasXSocialLinkPost; // 0x2d
    XOVERLAPPED mOverlapped; // 0x30
    DWORD mSocialCapabilities; // 0x4c
    int mSigninMask; // 0x50
    int mSigninChangeMask; // 0x54
    bool mGuideShowing; // 0x58
    bool mConfirmCancelSwapped; // 0x59
    bool mConnected; // 0x5a
    bool mScreenSaver; // 0x5b
    PlatformRegion mRegion; // 0x5c
    DiskError mDiskError; // 0x60
    JobMgr *mJobMgr; // 0x64
    bool unk68;
    bool unk69;

    DataNode OnSignInUsers(const DataArray *);
};

extern PlatformMgr ThePlatformMgr;

Symbol PlatformRegionToSymbol(PlatformRegion);
PlatformRegion SymbolToPlatformRegion(Symbol);

DECLARE_MESSAGE(PlatformMgrOpCompleteMsg, "platform_mgr_op_complete_msg")
PlatformMgrOpCompleteMsg(bool b1) : Message(Type(), b1) {}
int Success() const { return mData->Int(2); }
END_MESSAGE

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
StorageChangedMsg() : Message(Type()) {}
END_MESSAGE

DECLARE_MESSAGE(SmartGlassMsg, "smart_glass_msg")
SmartGlassMsg(int id, DataArray *a) : Message(Type(), id, a) {}
END_MESSAGE

DECLARE_MESSAGE(ControllerReqOpCompleteMsg, "controller_req_op_complete")
ControllerReqOpCompleteMsg(bool success) : Message(Type(), success) {}
void SetSuccess(bool success) { mData->Node(2) = success; }
END_MESSAGE

DECLARE_MESSAGE(InviteAcceptedMsg, "invite_accepted")
InviteAcceptedMsg(int i1, unsigned int i2, bool b3) : Message(Type(), i1, (int)i2, b3) {}
END_MESSAGE

DECLARE_MESSAGE(XMPStateChangedMsg, "xmp_state_changed")
XMPStateChangedMsg(int i) : Message(Type(), i) {}
bool Success() const { return mData->Int(2); }
END_MESSAGE

DECLARE_MESSAGE(PartyMembersChangedMsg, "party_members_changed")
PartyMembersChangedMsg() : Message(Type()) {}
END_MESSAGE
