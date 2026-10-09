#include "gesture/GestureMgr.h"
#include "meta/ConnectionStatusPanel.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
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
#include "xdk/xapilibi/xbox.h"
#include <algorithm>
#include <cstdlib>
#include <cwchar>

Hmx::Object *PlatformMgr::spShowControllerObject = nullptr;
DWORD PlatformMgr::sdwShowControllerTrackingID = 0;
int PlatformMgr::snShowControllerPadNum = 0;

#pragma region Anonymous Fields

namespace {
    enum ServiceIdState {
        kServiceIdStart = 0,
        kServiceIdEnumBegin = 1,
        kServiceIdEnum = 2,
        kServiceIdDownloadBegin = 3,
        kServiceIdDownload = 4,
        kServiceIdErrorTimeout = 5,
        kServiceIdEnd = 6,
    };

    class FriendEnumRequest {
    public:
        FriendEnumRequest(
            int padNum, std::vector<Friend *> *friends, Hmx::Object *callback
        )
            : mPadNum(padNum), mFriends(friends), mCallback(callback) {}
        MEM_OVERLOAD(FriendEnumRequest, 0x3E);

        int mPadNum;
        std::vector<Friend *> *mFriends;
        Hmx::Object *mCallback;
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
    wchar_t mStrStorageFiles[1][256];
    Hmx::Object *mFriendsCallback;
    void *mFriendsAsync; // XOVERLAPPED*
    void *mFriendsBuffer; // array of 0xc4 sized structs?
    void *mFriendsEnum;
    HANDLE mListener;
    int mSigninSameGuest;
    XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS mResults;
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

    DataArrayPtr JsonToDta(HJSONREADER *hReader, bool topLevel) {
        DataArrayPtr res;
        DataArray *pair = nullptr;
        JSONTokenType jsonTokenType;
        DWORD tokenLength;
        DWORD parsed;
        WCHAR valueBufferWC[128];
        char valueBuffer[256];
        while (XJSONReadToken(hReader, &jsonTokenType, &tokenLength, &parsed) == S_OK) {
            DataNode n;
            valueBuffer[0] = '\0';
            if (jsonTokenType >= Json_String
                && (jsonTokenType <= Json_Number || jsonTokenType == Json_FieldName)) {
                XJSONGetTokenValue(hReader, valueBufferWC, 128);
                wcstombs(valueBuffer, valueBufferWC, 256);
            }
            switch (jsonTokenType) {
            case Json_FieldName:
                pair = new DataArray(2);
                pair->Node(0) = Symbol(valueBuffer);
                continue;
            case Json_NotStarted:
                break;
            case Json_BeginArray:
                n = JsonToDta(hReader, false);
                break;
            case Json_EndArray:
                return res;
            case Json_BeginObject:
                n = JsonToDta(hReader, false);
                break;
            case Json_EndObject:
                return res;
            case Json_String:
                n = DataNode(valueBuffer);
                break;
            case Json_Number:
                if (strchr(valueBuffer, '.')) {
                    n = (float)atof(valueBuffer);
                } else {
                    n = atoi(valueBuffer);
                }
                break;
            case Json_True:
                n = 1;
                break;
            case Json_False:
                n = 0;
                break;
            case Json_Null:
                n = 0;
                break;
            case Json_NameSeparator:
                continue;
            case Json_ObjectSeparator:
                continue;
            case Json_ValueSeparator:
                continue;
            default:
                continue;
            }
            if (pair) {
                pair->Node(1) = n;
                n = pair;
                pair = nullptr;
            }
            if (topLevel && n.Type() == kDataArray) {
                res = n.Array();
                topLevel = false;
            } else {
                res->Insert(res->Size(), n);
            }
        }
        return res;
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
    mServiceIdState = kServiceIdStart;
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
        MILO_ASSERT(ret == ERROR_SUCCESS, 0x929);

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
    }
    BOOL result;
    XUserCheckPrivilege(padNum, XPRIVILEGE_MULTIPLAYER_SESSIONS, &result);
    return result;
}

ShowGamercardResult
PlatformMgr::ShowGamercardForPadNum(int padNum, const OnlineID *onlineID) {
    static GlitchAverager glAvg;
    AutoGlitchPoker poker(__FUNCTION__, 1, 0, &glAvg);
    MILO_ASSERT(onlineID, 0x7C6);
    if (!onlineID->GetIsValid()) {
        return (ShowGamercardResult)-1;
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
            return (ShowGamercardResult)-1;
        } else {
            return (ShowGamercardResult)0;
        }
    }
    return (ShowGamercardResult)-1;
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
    mFriendEnumRequests.push_back(new FriendEnumRequest(i1, &vec, o));
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
    mTime.Split();
    DWORD notificationID;
    ULONG_PTR param;
    static bool connectAfterSignin;
    while (XNotifyGetNext(mListener, 0, &notificationID, &param)) {
        switch (notificationID) {
        case 10: {
            UpdateSigninState();
            if (connectAfterSignin && mSigninMask) {
                mConnected = true;
                connectAfterSignin = false;
                ConnectionStatusChangedMsg msg(true);
                Handle(msg, false);
            }
            SigninChangedMsg msg(mSigninMask, mSigninChangeMask);
            Handle(msg, false);
            break;
        }
        case 0x2000001: {
            bool connected = mConnected;
            connectAfterSignin = false;
            mConnected = param == 0x1510f0;
            if (connected != mConnected) {
                if (mConnected && mSigninMask == 0) {
                    mConnected = false;
                    connectAfterSignin = true;
                } else {
                    ConnectionStatusChangedMsg msg(mConnected);
                    Handle(msg, false);
                }
            }
            break;
        }
        case 9: {
            mGuideShowing = param != 0;
            UIChangedMsg msg(mGuideShowing);
            Handle(msg, false);
            break;
        }
        case 11: {
            StorageChangedMsg msg;
            Handle(msg, false);
            break;
        }
        case 0x2000007: {
            ContentInstalledMsg msg;
            Handle(msg, false);
            break;
        }
        case 0x2000002: {
            InviteAcceptedMsg msg(param, 0, false);
            Handle(msg, false);
            break;
        }
        case 0xa000001: {
            XMPStateChangedMsg msg(param);
            Handle(msg, false);
            break;
        }
        case 0x4000002:
        case 0x4000003: {
            FriendsListChangedMsg msg(param);
            Handle(msg, false);
            break;
        }
        case 0xe040002: {
            PartyMembersChangedMsg msg;
            Handle(msg, false);
            break;
        }
        case 0x6001A: {
            static KinectGuideGestureMsg msg(0);
            msg[0] = param;
            Handle(msg, false);
            break;
        }
        case 0x60019: {
            int status = 2;
            if (param & 4)
                status = 2;
            else if (param & 2)
                status = 1;
            else if (param & 1)
                status = 0;
            KinectHardwareStatusMsg msg(status);
            Handle(msg, false);
            break;
        }
        case 0x6001D: {
            static KinectUserBindingChangedMsg msg(0);
            msg[0] = param;
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
        XOVERLAPPED *pFriendsAsync = (XOVERLAPPED *)mFriendsAsync;
        DWORD numFriends;
        DWORD res = XGetOverlappedResult(pFriendsAsync, &numFriends, false);
        if (res != ERROR_IO_INCOMPLETE) {
            static PlatformMgrOpCompleteMsg msg(false);
            if (res == ERROR_SUCCESS) {
                XONLINE_FRIEND *friends = (XONLINE_FRIEND *)mFriendsBuffer;
                for (DWORD i = 0; i < numFriends; i++) {
                    DWORD state = friends[i].dwFriendState;
                    if (!(state & XONLINE_FRIENDSTATE_FLAG_REQUEST)
                        && !(state & XONLINE_FRIENDSTATE_FLAG_PENDING)) {
                        Friend *newFriend = new Friend();
                        String name(friends[i].szGamertag);
                        newFriend->SetName(name);
                        newFriend->SetXUID(friends[i].xuid);
                        mFriendsList->push_back(newFriend);
                    }
                }
                msg[0] = true;
            } else {
                msg[0] = false;
            }
            mFriendsCallback->Handle(msg, true);
            mFriendsCallback = nullptr;
            mFriendsList = nullptr;
            RELEASE(mFriendsBuffer);
            RELEASE(mFriendsAsync);
            CloseHandle(mFriendsEnum);
            mFriendsEnum = nullptr;
        }
    } else if (mFriendEnumRequests.size() != 0) {
        FriendEnumRequest *req = mFriendEnumRequests.front();
        DWORD bufferSize;
        DWORD res =
            XFriendsCreateEnumerator(req->mPadNum, 0, 100, &bufferSize, &mFriendsEnum);
        bool failed = false;
        if (res == ERROR_SUCCESS) {
            MILO_ASSERT(!mFriendsBuffer, 0x503);
            mFriendsBuffer = new char[bufferSize];
            XOVERLAPPED *pFriendsAsync = new XOVERLAPPED;
            mFriendsAsync = pFriendsAsync;
            memset(pFriendsAsync, 0, sizeof(XOVERLAPPED));
            if (XEnumerate(
                    mFriendsEnum, mFriendsBuffer, bufferSize, nullptr, pFriendsAsync
                )
                != ERROR_IO_PENDING) {
                failed = true;
            }
        } else {
            failed = true;
        }
        if (failed) {
            if (mFriendsEnum) {
                CloseHandle(mFriendsEnum);
                mFriendsEnum = nullptr;
            }
            RELEASE(mFriendsBuffer);
            RELEASE(mFriendsAsync);
            static PlatformMgrOpCompleteMsg msg(false);
            req->mCallback->Handle(msg, true);
        } else {
            MILO_ASSERT(!mFriendsCallback, 0x52D);
            MILO_ASSERT(!mFriendsList, 0x52E);
            mFriendsCallback = req->mCallback;
            mFriendsList = req->mFriends;
        }
        delete req;
        mFriendEnumRequests.pop_front();
    }
    if (mServiceIdState != kServiceIdEnd) {
        switch (mServiceIdState) {
        case kServiceIdStart:
            mUserID = -1;
            for (int i = 0; i < 4; i++) {
                if (XUserGetSigninState(i) == eXUserSigninState_SignedInToLive) {
                    mUserID = i;
                    mResult = XStorageBuildServerPath(
                        i,
                        XSTORAGE_FACILITY_PER_TITLE,
                        nullptr,
                        0,
                        L"service_ids.dta",
                        mStrServerPath,
                        &mPathLen
                    );
                    if (mResult == ERROR_SUCCESS) {
                        mServiceIdState = kServiceIdEnumBegin;
                    } else {
                        mRetryTime = mTime.Ms() + 300000.0f;
                        mServiceIdState = kServiceIdErrorTimeout;
                    }
                    break;
                }
            }
            break;
        case kServiceIdEnumBegin:
            mStorageList = (XSTORAGE_ENUMERATE_RESULTS *)new BYTE[0x24B];
            if (mStorageList) {
                mServiceIDOverlapped = new XOVERLAPPED();
                mResult = XStorageEnumerate(
                    mUserID, mStrServerPath, 0, 1, 0x24B, mStorageList, mServiceIDOverlapped
                );
                if (mResult == ERROR_SUCCESS || mResult == ERROR_IO_PENDING) {
                    mServiceIdState = kServiceIdEnum;
                } else {
                    RELEASE(mStorageList);
                    RELEASE(mServiceIDOverlapped);
                    mServiceIdState = kServiceIdErrorTimeout;
                    mRetryTime = mTime.Ms() + 300000.0f;
                }
            } else {
                mServiceIdState = kServiceIdErrorTimeout;
                mRetryTime = mTime.Ms() + 300000.0f;
            }
            break;
        case kServiceIdEnum: {
            DWORD res = XGetOverlappedResult(mServiceIDOverlapped, &mResult, false);
            if (res == ERROR_IO_INCOMPLETE) {
                break;
            }
            if (res == ERROR_SUCCESS && mStorageList->dwNumItemsReturned != 0) {
                for (DWORD i = 0; i < mStorageList->dwNumItemsReturned; i++) {
                    swprintf_s(
                        mStrStorageFiles[i], L"%s", mStorageList->pItems[i].pwszPathName
                    );
                }
                mListSize = mStorageList->dwNumItemsReturned;
                mServiceIdState = kServiceIdDownloadBegin;
            } else {
                mRetryTime = mTime.Ms() + 300000.0f;
                mServiceIdState = kServiceIdErrorTimeout;
            }
            RELEASE(mStorageList);
            RELEASE(mServiceIDOverlapped);
            break;
        }
        case kServiceIdDownloadBegin:
            mServiceIDOverlapped = new XOVERLAPPED();
            mResult = XStorageDownloadToMemory(
                mUserID,
                mStrStorageFiles[0],
                sizeof(mFileReadBuffer),
                (BYTE *)mFileReadBuffer,
                sizeof(XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS),
                &mResults,
                mServiceIDOverlapped
            );
            if (mResult == ERROR_SUCCESS || mResult == ERROR_IO_PENDING) {
                mServiceIdState = kServiceIdDownload;
            } else {
                RELEASE(mServiceIDOverlapped);
                mServiceIdState = kServiceIdErrorTimeout;
                mRetryTime = mTime.Ms() + 300000.0f;
            }
            break;
        case kServiceIdDownload: {
            DWORD res = XGetOverlappedResult(mServiceIDOverlapped, &mResult, false);
            if (res == ERROR_IO_INCOMPLETE) {
                break;
            }
            if (res == ERROR_SUCCESS) {
                static Symbol service_ids("service_ids");
                DataArray *ids =
                    DataReadString((char *)mFileReadBuffer)->FindArray(service_ids, true);
                int numIds = ids->Size();
                mServiceIdMap.clear();
                for (int i = 1; i < numIds; i++) {
                    DataArray *entry = ids->Array(i);
                    String tmpServiceIdStr(entry->Str(0));
                    unsigned int id = entry->Int(1);
                    mServiceIdMap.insert(std::make_pair(tmpServiceIdStr, id));
                }
            }
            mServiceIdState = kServiceIdEnd;
            RELEASE(mServiceIDOverlapped);
            TmsDownloadedMsg msg;
            Handle(msg, false);
            break;
        }
        case kServiceIdErrorTimeout:
            if (mTime.Ms() >= mRetryTime) {
                mServiceIdState = kServiceIdStart;
            }
            break;
        default:
            MILO_FAIL("Invalid state!");
            break;
        }
    }
}

#pragma endregion
