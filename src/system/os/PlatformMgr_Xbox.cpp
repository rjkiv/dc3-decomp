#include "gesture/GestureMgr.h"
#include "meta/ConnectionStatusPanel.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/Friend.h"
#include "os/NetworkSocket_Win.h"
#include "os/OnlineID.h"
#include "os/PlatformMgr.h"
#include "os/ThreadCall.h"
#include "stdlib.h"
#include "ui/UI.h"
#include "utl/DataPointMgr.h"
#include "utl/GlitchFinder.h"
#include "utl/JobMgr.h"
#include "utl/Locale.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include "xdk/XAPILIB.h"
#include "xdk/XMP.h"
#include "xdk/XNET.h"
#include "xdk/XONLINE.h"
#include "xdk/XPARTY.h"
#include "xdk/NUI.h"
#include "xdk/XBC.h"
#include "xdk/win_types.h"
#include <algorithm>
#include <cstdlib>
#include <cwchar>

Hmx::Object *PlatformMgr::spShowControllerObject = nullptr;
DWORD PlatformMgr::sdwShowControllerTrackingID = 0;
int PlatformMgr::snShowControllerPadNum = 0;

#pragma region Anonymous Fields

namespace {
    enum ServiceIdState {
    };

    class FriendEnumRequest {
    public:
        FriendEnumRequest(int i, std::vector<Friend *> &f, Hmx::Object *o)
            : unk0(i), unk4(f), unk8(o) {}
        MEM_OVERLOAD(FriendEnumRequest, 0x3E);

        int unk0;
        std::vector<Friend *> &unk4;
        Hmx::Object *unk8;
    };

    DWORD gSmartGlassClientIDs[4];
    XUID mXuidCache[4];
    unsigned long mResult;
    unsigned long mUserID;
    unsigned long mPathLen;
    unsigned long mListSize;
    unsigned long mFileReadBuffer[1024];
    wchar_t mStrServerPath[512];
    XSTORAGE_ENUMERATE_RESULTS *mStorageList;
    XOVERLAPPED *mServiceIDOverlapped2;
    XOVERLAPPED *mServiceIDOverlapped;
    ServiceIdState mServiceIdState;
    float mRetryTime;
    std::vector<Friend *> *mFriendsList;
    WCHAR (*mStrStorageFiles)[256];
    Hmx::Object *mFriendsCallback;
    void *mFriendsAsync; // XOVERLAPPED*
    void *mFriendsBuffer; // array of 0xc4 sized structs?
    void *mFriendsEnum;
    HANDLE mListener;
    int mSigninSameGuest;
    // ...
    // struct _XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS `anonymous namespace'::mResults
    int gNumSmartGlassClients;
    int gNumSmartGlassSendsInProgress;
    std::list<FriendEnumRequest *> mFriendEnumRequests;
    Timer mTime;
    std::map<String, unsigned int> mServiceIdMap;

    int GetPadNumFromXuid(XUID xuid) {
        for (int i = 0; i < 4; i++) {
            XUSER_SIGNIN_INFO info;
            memset(&info, 0, sizeof(XUSER_SIGNIN_INFO));
            info.xuid = xuid;
            XUserGetSigninInfo(i, 1, &info);
            if (info.xuid == 0) {
                return i;
            }
            memset(&info, 0, sizeof(XUSER_SIGNIN_INFO));
            info.xuid = xuid;
            XUserGetSigninInfo(i, 2, &info);
            if (info.xuid == 0) {
                return i;
            }
            memset(&info, 0, sizeof(XUSER_SIGNIN_INFO));
            XUserGetXUID(i, &info.xuid);
            if (info.xuid == 0) {
                return i;
            }
        }
        return -1;
    }

    bool XPrivilegeCheck(XPRIVILEGE_TYPE t1, XPRIVILEGE_TYPE t2, XUID xuid) {
        BOOL result = false;
        XUserCheckPrivilege(0xFF, t1, &result);
        if (!result) {
            XUserCheckPrivilege(0xFF, t2, &result);
            if (!result) {
                return false;
            }
            for (int i = 0; i < 4; i++) {
                if (XUserCheckPrivilege(i, t1, &result) == 0 && !result
                    && XUserAreUsersFriends(i, &xuid, 1, &result, nullptr) == 0
                    && !result) {
                    return false;
                }
            }
        }
        return true;
    }

    void DtaToJsonHelper(HJSONWRITER *writer, const DataArray *a) {
        int aSize = a->Size();
        if (aSize != 0) {
            for (int i = 0; i < aSize; i++) {
                const DataNode &n = a->Node(i);
                switch (n.Type()) {
                case kDataInt:
                    XJSONWriteNumberValue(writer, n.Int());
                    break;
                case kDataFloat:
                    XJSONWriteNumberValue(writer, n.Float());
                    break;
                case kDataSymbol:
                    XJSONWriteStringValue(writer, n.Sym().Str(), strlen(n.Sym().Str()));
                    break;
                case kDataArray:
                    XJSONBeginArray(writer);
                    DtaToJsonHelper(writer, n.Array());
                    XJSONEndArray(writer);
                    break;
                case kDataString:
                    XJSONWriteStringValue(writer, n.Str(), strlen(n.Str()));
                    break;
                default:
                    MILO_NOTIFY("DtaToJson can't handle type %d right now", n.Type());
                    XJSONWriteNullValue(writer);
                    break;
                }
            }
        }
    }

    DataArrayPtr JsonToDta(HJSONREADER *reader, bool b2) {
        DataArrayPtr ptr;
        DataArray *arr = nullptr;
        JSONTokenType tokenType;
        DWORD tokenLength;
        DWORD parsed;
        WCHAR src[128];
        char dest[256];
        while (XJSONReadToken(reader, &tokenType, &tokenLength, &parsed) == 0) {
            DataNode curNode;
            dest[0] = 0;
            if (tokenType >= 5 && (tokenType <= 6 || tokenType == 10)) {
                XJSONGetTokenValue(reader, src, 128);
                wcstombs(dest, src, 256);
            }
            switch (tokenType) {
            case 0:
            case0:
                if (arr) {
                    arr->Node(1) = curNode;
                    curNode = arr;
                    arr = nullptr;
                }
                if (b2 && curNode.Type() == kDataArray) {
                    ptr = curNode.Array();
                    b2 = false;
                } else {
                    ptr->Insert(ptr->Size(), curNode);
                }
                break;
            case 1:
                curNode = JsonToDta(reader, false);
                goto case0;
                break;
            case 2:
                ptr->AddRef();
                break;
            case 3:
                curNode = JsonToDta(reader, false);
                goto case0;
                break;
            case 4:
                ptr->AddRef();
                break;
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
                break;
            }
        }
        return ptr;
    }

    HJSONWRITER *DtaToJson(const DataArray *a) {
        HJSONWRITER *writer = XJSONCreateWriter();
        XJSONBeginArray(writer);
        DtaToJsonHelper(writer, a);
        XJSONEndArray(writer);
        return writer;
    }

    void XbcSendMsg(DWORD id, const DataArray *a) {
        HJSONWRITER *writer = DtaToJson(a);
        if (id == 0) {
            // i hate this
            for (DWORD *it = gSmartGlassClientIDs; it < &gSmartGlassClientIDs[4]; it++) {
                if (*it) {
                    XbcSendJSON(XBC_DELIVERY_RELIABLE, *it, writer, nullptr);
                    gNumSmartGlassSendsInProgress++;
                }
            }
        } else {
            XbcSendJSON(XBC_DELIVERY_RELIABLE, id, writer, nullptr);
            gNumSmartGlassSendsInProgress++;
        }
        XJSONCloseWriter(writer);
    }

    void XbcRecieveMsg(DWORD id, HJSONREADER *reader) {
        DataArrayPtr dta = JsonToDta(reader, true);
        SmartGlassMsg msg(id, dta);
        ThePlatformMgr.Handle(msg, true);
    }

    void XbcCallback(long err, XBC_EVENT_PARAMS *params, void *) {
        if (err != 0) {
            MILO_NOTIFY("SmartGlass: Error in cb: 0x%08x", err);
        } else if (params->nUserIndex >= 4) {
            MILO_NOTIFY(
                "SmartGlass: Error in cb: user index %d (event: %d)",
                params->nUserIndex,
                params->Type
            );
        } else {
            switch (params->Type) {
            case XBC_EVENT_CLIENT_CONNECTED: {
                gSmartGlassClientIDs[params->nUserIndex] = params->nClientId;
                gNumSmartGlassClients++;
                MILO_ASSERT(gNumSmartGlassClients <= XBC_MAX_CLIENTS, 0x20C);
                break;
            }
            case XBC_EVENT_CLIENT_DISCONNECTED: {
                gSmartGlassClientIDs[params->nUserIndex] = 0;
                gNumSmartGlassClients--;
                MILO_ASSERT(gNumSmartGlassClients >= 0, 0x214);
                break;
            }
            case XBC_EVENT_JSON_SEND_COMPLETE: {
                gNumSmartGlassSendsInProgress--;
                break;
            }
            case XBC_EVENT_JSON_RECEIVE_COMPLETE: {
                XbcRecieveMsg(params->nClientId, params->hReader);
                break;
            }
            default:
                break;
            }
        }
    }

    void SmartGlassInit() {
        for (int i = 0; i < 4; i++) {
            gSmartGlassClientIDs[i] = 0;
        }
        if (XbcInitialize(XbcCallback, nullptr) < 0) {
            MILO_FAIL("Failed to initialize Xbox SmartGlass library.\n");
        }
    }

    void SmartGlassPoll() {
        HRESULT res = XbcDoWork();
        if (res != 0) {
            MILO_NOTIFY("SmartGlass: error: %d\n", res);
        }
    }

}

#pragma endregion
#pragma region Jobs

SingleItemEnumJob::SingleItemEnumJob(Hmx::Object *callback, int pad, QWORD offerID)
    : mCallback(callback), mPadNum(pad), mOfferID(offerID), mState(0), unk1c(false),
      unk20(0), mEnum(0) {}

SingleItemEnumJob::~SingleItemEnumJob() {
    if (mState == 1 && mOverlapped.InternalLow == ERROR_IO_PENDING) {
        DWORD res = XCancelOverlapped(&mOverlapped);
        if (res != 0) {
            MILO_FAIL("Error cancelling enum %d", res);
        }
    }
    if (mEnum) {
        CloseHandle(mEnum);
        mEnum = nullptr;
    }
    RELEASE(unk20);
}

void SingleItemEnumJob::Start() {
    DWORD size = 0;
    mState = 1;
    DWORD res = XMarketplaceCreateOfferEnumeratorByOffering(
        mPadNum, 1, &mOfferID, 1, &size, &mEnum
    );
    if (res != 0) {
        if (mEnum) {
            CloseHandle(mEnum);
            mEnum = nullptr;
        }
        MILO_NOTIFY("Error creating enumerator after purchase: %d", res);
        mState = 3;
    } else {
        unk20 = new char[size];
        memset(unk20, 0, size);
        memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
        DWORD enumRes = XEnumerate(mEnum, unk20, size, nullptr, &mOverlapped);
        if (enumRes != ERROR_IO_PENDING) {
            if (mEnum) {
                CloseHandle(mEnum);
                mEnum = nullptr;
            }
            RELEASE(unk20);
            MILO_NOTIFY("Error enumerating after purchase: %d", enumRes);
            mState = 3;
        }
    }
}

bool SingleItemEnumJob::IsFinished() {
    if (mState == 1) {
        Poll();
    }
    return mState != 1;
}

void SingleItemEnumJob::Cancel(Hmx::Object *) {
    MILO_FAIL("SingleItemEnumJob::Cancel called");
}

void SingleItemEnumJob::OnCompletion(Hmx::Object *) {
    if (mCallback) {
        static SingleItemEnumCompleteMsg msg(false, false, gNullStr);
        msg.SetSuccess(mState == 2);
        msg.SetPurchaseMade(unk1c);
        String offerID = MakeString("%016llX", mOfferID);
        msg.SetOfferID(offerID);
        mCallback->Handle(msg, true);
    }
}

void SingleItemEnumJob::Poll() {
    if (mState == 1 && mOverlapped.InternalLow != ERROR_IO_PENDING) {
        DWORD dwResult;
        DWORD res = XGetOverlappedResult(&mOverlapped, &dwResult, false);
        if (dwResult == 1 && res == 0) {
            mState = 2;
            // unk1c =
        } else {
            mState = 3;
            MILO_NOTIFY("Error enumerating after purchase: %d", res);
        }
        if (mEnum) {
            CloseHandle(mEnum);
            mEnum = nullptr;
        }
        RELEASE(unk20);
    }
}

PostPurchaseEnumJob::PostPurchaseEnumJob(
    Hmx::Object *callback, int pad, QWORD offerID, Symbol src, unsigned int purchaser
)
    : SingleItemEnumJob(callback, pad, offerID), mSource(src), mPurchaser(purchaser) {}

void PostPurchaseEnumJob::OnCompletion(Hmx::Object *o) {
    if (mState == 2) {
        if (unk1c) {
            static Symbol source("source");
            static Symbol offer("offer");
            static Symbol purchaser("purchaser");
            String offerStr(MakeString("%016llX", mOfferID));
            SendDataPoint(
                "store/purchase",
                source,
                mSource,
                offer,
                offerStr.c_str(),
                purchaser,
                (int)mPurchaser
            );
        }
    }
    SingleItemEnumJob::OnCompletion(o);
}

MultipleItemsEnumJob::MultipleItemsEnumJob(
    Hmx::Object *callback, int pad, std::vector<QWORD> &ids
)
    : mCallback(callback), mPadNum(pad), mOfferIDs(ids), mState(0), unk34(false),
      unk38(0), mEnum(0) {}

MultipleItemsEnumJob::~MultipleItemsEnumJob() {
    if (mState == 1 && mOverlapped.InternalLow == ERROR_IO_PENDING) {
        DWORD res = XCancelOverlapped(&mOverlapped);
        if (res != 0) {
            MILO_FAIL("Error cancelling enum %d", res);
        }
    }
    if (mEnum) {
        CloseHandle(mEnum);
        mEnum = nullptr;
    }
    RELEASE(unk38);
}

void MultipleItemsEnumJob::Start() {
    mState = 1;
    unk1c.resize(mOfferIDs.size());
    std::fill(unk1c.begin(), unk1c.end(), false);
    DWORD size = 0;
    int numItems = mOfferIDs.size();
    DWORD res = XMarketplaceCreateOfferEnumeratorByOffering(
        mPadNum, numItems, mOfferIDs.begin(), numItems, &size, &mEnum
    );
    if (res != 0) {
        if (mEnum) {
            CloseHandle(mEnum);
            mEnum = nullptr;
        }
        MILO_NOTIFY("Error creating enumerator after purchase: %d", res);
        mState = 3;
    } else {
        unk38 = new char[size];
        memset(unk38, 0, size);
        memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
        DWORD enumRes = XEnumerate(mEnum, unk38, size, nullptr, &mOverlapped);
        if (enumRes != ERROR_IO_PENDING) {
            if (mEnum) {
                CloseHandle(mEnum);
                mEnum = nullptr;
            }
            RELEASE(unk38);
            MILO_NOTIFY("Error enumerating after purchase: %d", enumRes);
            mState = 3;
        }
    }
}

bool MultipleItemsEnumJob::IsFinished() {
    if (mState == 1) {
        Poll();
    }
    return mState != 1;
}

void MultipleItemsEnumJob::Cancel(Hmx::Object *) {
    MILO_FAIL("MultipleItemsEnumJob::Cancel called");
}

void MultipleItemsEnumJob::OnCompletion(Hmx::Object *) {
    if (mCallback) {
        static MultipleItemsEnumCompleteMsg msg(false, false, mOfferIDs.size(), gNullStr);
        msg.SetSuccess(mState == 2);
        msg.SetPurchaseMade(unk34);
        int numIDs = mOfferIDs.size();
        msg.SetNumOfferIDs(numIDs);
        for (int i = 0; i < numIDs; i++) {
            String curID = MakeString("%016llX", mOfferIDs[i]);
            msg.SetOfferID(i, curID);
            msg.SetPurchased(i, unk1c[i]);
        }
        mCallback->Handle(msg, true);
    }
}

void MultipleItemsEnumJob::Poll() {
    if (mState == 1 && mOverlapped.InternalLow != ERROR_IO_PENDING) {
        DWORD dwResult;
        DWORD res = XGetOverlappedResult(&mOverlapped, &dwResult, false);
        if (res == 0) {
            mState = 2;
            for (int i = 0; i < mOfferIDs.size(); i++) {
                // stuff happens here
            }
        } else {
            mState = 3;
            MILO_NOTIFY("Error enumerating after purchase: %d", res);
        }
        if (mEnum) {
            CloseHandle(mEnum);
            mEnum = nullptr;
        }
        RELEASE(unk38);
    }
}

MultipleItemsPostPurchaseEnumJob::MultipleItemsPostPurchaseEnumJob(
    Hmx::Object *callback,
    int pad,
    std::vector<QWORD> &offerIDs,
    Symbol src,
    unsigned int purchaser
)
    : MultipleItemsEnumJob(callback, pad, offerIDs), mSource(src), mPurchaser(purchaser) {
}

void MultipleItemsPostPurchaseEnumJob::OnCompletion(Hmx::Object *o) {
    if (mState == 2) {
        if (unk34) {
            static Symbol source("source");
            static Symbol offer("offer");
            static Symbol purchaser("purchaser");
            for (int i = 0; i < mOfferIDs.size(); i++) {
                String curID(MakeString("%016llX", mOfferIDs[i]));
                SendDataPoint(
                    "store/purchase",
                    source,
                    mSource,
                    offer,
                    curID.c_str(),
                    purchaser,
                    (int)mPurchaser
                );
            }
        }
    }
    MultipleItemsEnumJob::OnCompletion(o);
}

QWORD SingleItemEnumCompleteMsg::OfferID() const {
    return _strtoui64(mData->Str(4), 0, 16);
}

QWORD MultipleItemsEnumCompleteMsg::OfferID(int i) const {
    return _strtoui64(mData->Array(5)->Str(i), 0, 16);
}

#pragma endregion
#pragma region PlatformMgr

PlatformMgr::PlatformMgr() : mSigninMask(0) {
    mScreenSaver = true;
    mSigninChangeMask = 0;
    mGuideShowing = false;
    mConfirmCancelSwapped = false;
    mConnected = false;
    mRegion = kRegionNone;
    mDiskError = kNoDiskError;
    unk69 = false;
    mSigninSameGuest = 0;
    mFriendsEnum = nullptr;
    mFriendsBuffer = nullptr;
    mFriendsCallback = nullptr;
    mFriendsAsync = nullptr;
    mFriendsList = nullptr;
    mListener = nullptr;
    mJobMgr = new JobMgr(this);
    for (int i = 0; i < 4; i++) {
        mXuidCache[i] = 0;
    }
    mServiceIDOverlapped = nullptr;
    mServiceIDOverlapped2 = nullptr;
    mStorageList = nullptr;
    mPathLen = 0x200;
    mServiceIdState = (ServiceIdState)0;
    mListSize = 0;
    mUserID = -1;
    mResult = 0;
    mOverlapped.hEvent = nullptr;
}

PlatformMgr::~PlatformMgr() {
    DWORD ret = CloseHandle(mListener);
    MILO_ASSERT(ret == ERROR_SUCCESS, 999);
    ret = XOnlineCleanup();
    MILO_ASSERT(ret == ERROR_SUCCESS, 0x3EA);
}

void PlatformMgr::PreInit() { XMPOverrideBackgroundMusic(); }

void PlatformMgr::Init() {
    SetName("platform_mgr", ObjectDir::Main());
    WinSockSocket::Init();
    DWORD ret = XOnlineStartup();
    MILO_ASSERT(ret == ERROR_SUCCESS, 0x419);
    mListener = XNotifyCreateListener(0xA7);
    MILO_ASSERT(mListener, 0x41C);
    UpdateSigninState();
    SmartGlassInit();
    mTime.Start();
    mRetryTime = mTime.Ms() + 300000.0f;
}

bool PlatformMgr::IsEthernetCableConnected() { return XNetGetEthernetLinkStatus() != 0; }

void PlatformMgr::UpdateSigninState() {
    XUID oldCache[4] = { mXuidCache[0], mXuidCache[1], mXuidCache[2], mXuidCache[3] };
    mSigninSameGuest = 0;
    mSigninChangeMask = 0;
    mSigninMask = 0;
    for (int i = 0; i < 4; i++) {
        if (XUserGetSigninState(i) == eXUserSigninState_NotSignedIn) {
            mXuidCache[i] = 0;
        } else {
            XUSER_SIGNIN_INFO info;
            memset(&info, 0, sizeof(XUSER_SIGNIN_INFO));
            mSigninMask |= 1 << i;
            XUserGetSigninInfo(i, 2, &info);
            XUserGetXUID(i, &info.xuid);
            mXuidCache[i] = 0;
        }
        if (oldCache[i] != mXuidCache[i]) {
            mSigninChangeMask |= 1 << i;
            if (((mXuidCache[i] ^ oldCache[i]) & 0xff3fffffffffffff) == 0) {
                mSigninSameGuest |= 1 << i;
            }
        }
    }
}

bool PlatformMgr::HasCreatedContentPrivilege() const {
    bool ret = true;
    for (int i = 0; i < 4; i++) {
        BOOL result = false;
        bool created =
            XUserCheckPrivilege(i, XPRIVILEGE_USER_CREATED_CONTENT, &result) == 0
            && result == 0;
        bool createdFriends =
            XUserCheckPrivilege(i, XPRIVILEGE_USER_CREATED_CONTENT_FRIENDS_ONLY, &result)
                == 0
            && result == 0;
        bool bAnd = !created || !createdFriends;
        ret &= bAnd;
    }
    return ret;
}

bool PlatformMgr::HasKinectSharePrvilege() const {
    BOOL result = false;
    return XUserCheckPrivilege(0xFF, XPRIVILEGE_SHARE_CONTENT_OUTSIDE_LIVE, &result) == 0
        && result;
}

bool PlatformMgr::IsSmartGlassConnected() { return gNumSmartGlassClients > 0; }

void PlatformMgr::SetPadContext(int padNum, int id, int val) const {
    if (padNum != -1 && ThePlatformMgr.IsSignedIn(padNum)) {
        XUserSetContext(padNum, id, val);
    }
}

void PlatformMgr::SetPadPresence(int padNum, int val) const {
    if (padNum != -1 && ThePlatformMgr.IsSignedIn(padNum)) {
        XUserSetContext(padNum, 0x8001, val);
    }
}

void PlatformMgr::ShowFriendsUI(int padNum) {
    DWORD trackingID;

    if (IsSignedIn(padNum)) {
        if (sXShowCallback(trackingID)) {
            XShowNuiFriendsUI(trackingID, padNum);
        } else {
            XShowFriendsUI(padNum);
        }
    }
}

void PlatformMgr::SetBackgroundDownloadPriority(bool alwaysAllow) {
    XBackgroundDownloadSetMode(
        alwaysAllow ? XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW
                    : XBACKGROUND_DOWNLOAD_MODE_AUTO
    );
}

void PlatformMgr::SignInUsers(int count, DWORD flags) {
    MILO_ASSERT(count == 1 || count == 2 || count == 4, 0x64E);
    DWORD trackingID;
    if (sXShowCallback(trackingID)) {
        XShowNuiSigninUI(trackingID, flags);
    } else {
        XShowSigninUI(count, flags);
    }
}

bool PlatformMgr::PollXSocialCapabilities() {
    if (mOverlapped.hEvent && mOverlapped.InternalLow != ERROR_IO_PENDING) {
        CloseHandle(mOverlapped.hEvent);
        mOverlapped.hEvent = nullptr;
        BOOL result = false;
        mHasXSocialPhotoPost = (unsigned char)mSocialCapabilities & 1;
        mHasXSocialLinkPost = ((unsigned char)mSocialCapabilities >> 1) & 1;
        if (XUserCheckPrivilege(0xFF, XPRIVILEGE_SOCIAL_NETWORK_SHARING, &result)
            || !result) {
            mHasXSocialPhotoPost = false;
            mHasXSocialLinkPost = false;
        }
        MILO_LOG(
            "PollXSocialCapabilities() - can post Photo:%s can post Link:%s\n",
            mHasXSocialPhotoPost ? "YES" : "NO",
            mHasXSocialLinkPost ? "YES" : "NO"
        );
        return true;
    } else {
        return false;
    }
}

int ShowControllerRequiredUIThreaded() {
    return XShowNuiControllerRequiredUI(
        PlatformMgr::sdwShowControllerTrackingID, PlatformMgr::snShowControllerPadNum
    );
}

void ShowControllerRequiredUIThreadedCB(int i1) {
    static ControllerReqOpCompleteMsg msg(true);
    msg.SetSuccess(i1 == 0);
    if (PlatformMgr::spShowControllerObject) {
        PlatformMgr::spShowControllerObject->Handle(msg, false);
    }
}

bool PlatformMgr::ShowPartyUI(int padNum) {
    DWORD trackingID;
    DWORD ret = 1;

    if (IsSignedIn(padNum)) {
        if (sXShowCallback(trackingID)) {
            ret = XShowNuiPartyUI(trackingID, padNum);
        } else {
            ret = XShowPartyUI(padNum);
        }
    }

    return ret == 0;
}

bool PlatformMgr::ShowFitnessBodyProfileUI(int padNum) {
    DWORD trackingID;
    DWORD ret = 1;

    if (IsSignedIn(padNum)) {
        if (sXShowCallback(trackingID)) {
            ret = XShowNuiFitnessBodyProfileUI(trackingID, padNum);
        } else {
            ret = XShowFitnessBodyProfileUI(padNum);
        }
    }

    return ret == 0;
}

void PlatformMgr::EnableXMP() { XMPRestoreBackgroundMusic(); }
void PlatformMgr::DisableXMP() { XMPOverrideBackgroundMusic(); }

void PlatformMgr::SetScreenSaver(bool screensaver) {
    mScreenSaver = screensaver;
    XEnableScreenSaver(screensaver);
}

bool PlatformMgr::IsSignedIntoLive(int padNum) const {
    MILO_ASSERT(padNum >= 0, 0x671);

    if (!IsSignedIn(padNum)) {
        return false;
    } else {
        return (XUserGetSigninState(padNum) == eXUserSigninState_SignedInToLive);
    }
}

bool PlatformMgr::IsPadAGuest(int padNum) const {
    XUSER_SIGNIN_INFO signinInfo;

    DWORD ret = XUserGetSigninInfo(padNum, 0, &signinInfo);

    if (ret == ERROR_NO_SUCH_USER) {
        return IsSignedIn(padNum);
    } else {
        MILO_ASSERT(ret != ERROR_SUCCESS, 0x929);

        return signinInfo.dwInfoFlags >> 1 & 1;
    }
}

void PlatformMgr::ShowOfferUI(int padNum) {
    DWORD trackingID;
    DWORD ret;

    if (IsSignedIn(padNum)) {
        if (sXShowCallback(trackingID)) {
            ret = XShowNuiMarketplaceUI(
                trackingID,
                padNum,
                XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST_BACKGROUND,
                0,
                -1
            );
        } else {
            ret = XShowMarketplaceUI(
                padNum, XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST_BACKGROUND, 0, -1
            );
        }

        if (ret != ERROR_SUCCESS) {
            MILO_NOTIFY("XShowMarketplaceUI failed (0x%x)", ret);
        }
    }
}

DWORD PlatformMgr::ShowDeviceSelectorUI(
    DWORD userIndex,
    DWORD contentType,
    DWORD contentFlags,
    ULARGE_INTEGER bytesRequested,
    DWORD *deviceID,
    XOVERLAPPED *overlapped
) {
    DWORD trackingID;
    DWORD ret;

    if (sXShowCallback(trackingID)) {
        ret = XShowNuiDeviceSelectorUI(
            trackingID,
            userIndex,
            contentType,
            contentFlags,
            bytesRequested,
            deviceID,
            overlapped
        );
    } else {
        ret = XShowDeviceSelectorUI(
            userIndex, contentType, contentFlags, bytesRequested, deviceID, overlapped
        );
    }

    return ret;
}

void PlatformMgr::RegionInit() {
    if (XGetGameRegion() != 0xFF) {
        SetRegion(kRegionEurope);
    } else {
        SetRegion(kRegionNA);
    }
}

const char *PlatformMgr::GetName(int padnum) const {
    if (IsSignedIn(padnum)) {
        char name[16];
        int ret = XUserGetName(padnum, name, 16);
        if (ret == 0) {
            return MakeString(name);
        }
    }
    static Symbol player("player");
    return MakeString("%s %i", Localize(player, nullptr, TheLocale), padnum + 1);
}

void PlatformMgr::SmartGlassSend(DWORD id, const DataArray *a) { XbcSendMsg(id, a); }

bool PlatformMgr::QueryXSocialCapabilities() {
    mSocialCapabilities = 0;
    mOverlapped.InternalContext = 0;
    mOverlapped.InternalHigh = 0;
    mOverlapped.InternalLow = 0;
    mOverlapped.hEvent = CreateEventA(nullptr, true, false, "QueryXSocialCapabilities");
    if (!mOverlapped.hEvent) {
        MILO_LOG("mOverlapped.hEvent is null");
        return false;
    } else {
        mOverlapped.pCompletionRoutine = 0;
        mOverlapped.dwCompletionContext = 0;
        mOverlapped.dwExtendedError = 0;
        DWORD res = XSocialGetCapabilities(&mSocialCapabilities, &mOverlapped);
        if (res == 0) {
            MILO_LOG(
                "XSocialGetCapabilities() returns success - %x\n", mSocialCapabilities
            );
            BOOL result = false;
            mHasXSocialPhotoPost = (unsigned char)mSocialCapabilities & 1;
            mHasXSocialLinkPost = ((unsigned char)mSocialCapabilities >> 1) & 1;
            if (XUserCheckPrivilege(0xFF, XPRIVILEGE_SOCIAL_NETWORK_SHARING, &result)
                || result == 0) {
                mHasXSocialPhotoPost = false;
                mHasXSocialLinkPost = false;
            }
            return true;
        } else if (res == ERROR_IO_PENDING) {
            MILO_LOG("XSocialGetCapabilities() returns ERROR_IO_PENDING\n");
            return true;
        } else {
            if (mOverlapped.hEvent) {
                CloseHandle(mOverlapped.hEvent);
                mOverlapped.hEvent = nullptr;
            }
            return false;
        }
    }
}

void PlatformMgr::SetPadProperty(int pad, int i2, const unsigned short *us) const {
    if (pad != -1 && ThePlatformMgr.IsSignedIn(pad)) {
        int len = Min<int>(wcslen((wchar_t *)us) * 2, 126);
        XUserSetPropertyEx(pad, i2, len, us, nullptr);
    }
}

bool PlatformMgr::IsInParty() {
    XPARTY_USER_LIST list;
    if (IsSignedIntoLive(0) || IsSignedIntoLive(1) || IsSignedIntoLive(2)
        || IsSignedIntoLive(3)) {
        return XPartyGetUserList(&list) != 0x807D0003;
    } else {
        return false;
    }
}

bool PlatformMgr::IsInPartyWithOthers() {
    XPARTY_USER_LIST list;
    // i also hate this
    return IsInParty() && (XPartyGetUserList(&list), (int)list.dwUserCount > 1);
}

void PlatformMgr::InviteParty(int padNum) {
    MILO_ASSERT(IsInParty(), 0x87B);
    if (IsSignedIn(padNum)) {
        XPartySendGameInvites(padNum, nullptr);
    }
}

int PlatformMgr::GetOwnerOfGuest(int padNum) {
    MILO_ASSERT(padNum != -1, 0x8F9);
    XUSER_SIGNIN_INFO signinInfo;
    DWORD ret = XUserGetSigninInfo(padNum, 0, &signinInfo);
    if (ret == 0x525) {
        XUID xuid;
        DWORD xuidRes = XUserGetXUID(padNum, &xuid);
        if (xuidRes == 0) {
            return GetPadNumFromXuid(xuid & 0xff3fffffffffffff);
        } else {
            return -1;
        }
    } else {
        MILO_ASSERT(ret == ERROR_SUCCESS, 0x911);
        MILO_ASSERT(signinInfo.dwInfoFlags & XUSER_INFO_FLAG_GUEST, 0x912);
        return signinInfo.dwSponsorUserIndex;
    }
}

void PlatformMgr::SetNotifyUILocation(NotifyLocation loc) {
    switch (loc) {
    case 0:
        XNotifyPositionUI(9);
        break;
    case 1:
        XNotifyPositionUI(2);
        break;
    default:
        MILO_FAIL("Unknown NotifyLocation %d", loc);
        break;
    }
}

bool PlatformMgr::HasOnlinePrivilege(int padNum) const {
    static GlitchAverager glAvg;
    AutoGlitchPoker poker(__FUNCTION__, 1, 0, &glAvg);
    MILO_ASSERT(padNum >= 0, 0x693);
    if (!IsSignedIntoLive(padNum)) {
        return false;
    } else {
        BOOL result;
        XUserCheckPrivilege(padNum, XPRIVILEGE_MULTIPLAYER_SESSIONS, &result);
        return result;
    }
}

ShowGamercardResult
PlatformMgr::ShowGamercardForPadNum(int padNum, const OnlineID *onlineID) {
    static GlitchAverager glAvg;
    AutoGlitchPoker poker(__FUNCTION__, 1, 0, &glAvg);
    MILO_ASSERT(onlineID, 0x7C6);
    if (!onlineID->GetIsValid()) {
        return kGamercardResultPrivelegeError;
    }
    if (!IsSignedIntoLive(padNum)) {
        return (ShowGamercardResult)-3;
    } else {
        XUID xuid = onlineID->GetXUID();
        if (!XPrivilegeCheck(
                XPRIVILEGE_PROFILE_VIEWING, XPRIVILEGE_PROFILE_VIEWING_FRIENDS_ONLY, xuid
            )) {
            return (ShowGamercardResult)-2;
        }
        DWORD dw;
        DWORD i4;
        if (sXShowCallback(dw)) {
            i4 = XShowNuiGamerCardUI(dw, padNum, xuid);
        } else {
            i4 = XShowGamerCardUI(padNum, xuid);
        }
        if (i4 != 0) {
            return kGamercardResultPrivelegeError;
        } else {
            return (ShowGamercardResult)0;
        }
    }
    return kGamercardResultPrivelegeError;
}

void PlatformMgr::ShowControllerRequiredUI(Hmx::Object *o1) {
    sdwShowControllerTrackingID = 0;
    snShowControllerPadNum = 0;
    spShowControllerObject = nullptr;
    DWORD dw;
    if (sXShowCallback(dw)) {
        sdwShowControllerTrackingID = dw;
        snShowControllerPadNum = 0xFF;
        spShowControllerObject = o1;
        ThreadCall(ShowControllerRequiredUIThreaded, ShowControllerRequiredUIThreadedCB);
    } else {
        static ControllerReqOpCompleteMsg msg(true);
        msg.SetSuccess(true);
        if (o1) {
            o1->Handle(msg, false);
        }
    }
}

void PlatformMgr::EnumerateFriends(int i1, std::vector<Friend *> &vec, Hmx::Object *o) {
    mFriendEnumRequests.push_back(new FriendEnumRequest(i1, vec, o));
}

bool PlatformMgr::GetServiceID(const String &str, unsigned int &ui) {
    ui = 0;
    bool ret = false;
    auto it = mServiceIdMap.find(str);
    if (it != mServiceIdMap.end()) {
        ui = it->second;
        ret = true;
    }
    return ret;
}

DataNode PlatformMgr::OnSignInUsers(const DataArray *a) {
    int i3 = 0;
    if (a->Size() > 3) {
        if (a->Int(3)) {
            i3 = 2;
        }
    }
    SignInUsers(a->Int(2), i3);
    return 0;
}

void PlatformMgr::Poll() {
    SmartGlassPoll();
    mJobMgr->Poll();
    mTime.Pause();
    DWORD dwId;
    ULONG param;
    static bool sb4;
    while (XNotifyGetNext(mListener, 0, &dwId, &param)) {
        switch (dwId) {
        case 9: {
            mGuideShowing = param == 0;
            UIChangedMsg msg(mGuideShowing);
            Handle(msg, false);
            break;
        }
        case 10: {
            UpdateSigninState();
            if (sb4 && mSigninMask) {
                mConnected = true;
                sb4 = false;
                ConnectionStatusChangedMsg msg(true);
                Handle(msg, false);
            }
            SigninChangedMsg msg(mSigninMask, mSigninChangeMask);
            Handle(msg, false);
            break;
        }
        case 11: {
            StorageChangedMsg msg;
            Handle(msg, false);
            break;
        }
        case 0x60019: {
            int u17;
            if (param & 4) {
                u17 = 2;
            } else if (param & 2) {
                u17 = 1;
            } else if (param & 1) {
                u17 = 0;
            } else {
                u17 = 2;
            }
            KinectHardwareStatusMsg msg(u17);
            Handle(msg, false);
            break;
        }
        case 1: {
            static KinectGuideGestureMsg msg(0);
            msg[0] = param;
            Handle(msg, false);
            break;
        }
        case 4: {
            static KinectUserBindingChangedMsg msg(0);
            msg[0] = param;
            Handle(msg, false);
            break;
        }
        case 0x2000001: {
            bool oldConnected = mConnected;
            sb4 = false;
            mConnected = param == 0x1510f0;
            if (oldConnected != mConnected) {
                if (mConnected && mSigninMask == 0) {
                    mConnected = false;
                    sb4 = true;
                } else {
                    ConnectionStatusChangedMsg msg(mConnected);
                    Handle(msg, false);
                }
            }
            break;
        }
        case 0x2000002: {
            InviteAcceptedMsg msg(param, 0, false);
            Handle(msg, false);
            break;
        }
        case 0x2000007: {
            ContentInstalledMsg msg;
            Handle(msg, false);
            break;
        }
        case 0x4000002:
        case 0x4000003: {
            FriendsListChangedMsg msg(param);
            Handle(msg, false);
            break;
        }
        case 0xa000001: {
            XMPStateChangedMsg msg(param);
            Handle(msg, false);
            break;
        }
        case 0xe040002: {
            PartyMembersChangedMsg msg;
            Handle(msg, false);
            break;
        }
        default:
            break;
        }
    }
    if (mFriendsEnum) {
        MILO_ASSERT(mFriendsBuffer, 0x4BE);
        MILO_ASSERT(mFriendsCallback, 0x4BF);
        MILO_ASSERT(mFriendsAsync, 0x4C0);
        MILO_ASSERT(mFriendsList, 0x4C1);
        DWORD result;
        DWORD res = XGetOverlappedResult((XOVERLAPPED *)mFriendsAsync, &result, false);
        if (res != ERROR_IO_INCOMPLETE) {
            static PlatformMgrOpCompleteMsg msg(false);
            if (res == ERROR_SUCCESS) {
                for (int i = 0; i < result; i++) {
                    // do a thing with mFriendsBuffer[i]
                }
                msg[0] = true;
            } else {
                msg[0] = false;
            }
            mFriendsCallback->Handle(msg, true);
        }
        mFriendsCallback = nullptr;
        mFriendsList = nullptr;
        RELEASE(mFriendsBuffer);
        RELEASE(mFriendsAsync);
        CloseHandle(mFriendsEnum);
        mFriendsEnum = nullptr;
    } else {
        if (!mFriendEnumRequests.empty() && mFriendEnumRequests.size() != 0) {
            // stuff and things
        }
    }
}

#pragma endregion
