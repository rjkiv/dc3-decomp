#include "meta/StorePurchaser.h"
#include "meta/StoreOffer.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "ui/UI.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"
#include <cstring>

#pragma region XboxPurchaser

XboxPurchaser::XboxPurchaser(
    int param1,
    unsigned long long param2,
    unsigned long long param3,
    unsigned long long param4,
    Symbol s,
    unsigned int ui
)
    : StorePurchaser(s, ui), mState(kIdle), mOfferID(param2), mPadNum(param1) {}

XboxPurchaser::~XboxPurchaser() {
    static Symbol ui_changed("ui_changed");
    ThePlatformMgr.RemoveSink(this, ui_changed);
}

void XboxPurchaser::Initiate() {
    MILO_ASSERT(!IsPurchasing(), 0x39a);
    mState = kPurchasing;
    DWORD result;
    DWORD id;
    if (PlatformMgr::sXShowCallback(id)) {
        result = XShowNuiMarketplaceUI(id, mPadNum, 5, mOfferID, -1);
    } else {
        result = XShowMarketplaceUI(mPadNum, 5, mOfferID, -1);
    }

    if (result != 0) {
        MILO_NOTIFY("Error starting checkout UI: %d", result);
        mState = kFailure;
    }

    static Symbol ui_changed("ui_changed");
    ThePlatformMgr.AddSink(this, ui_changed);
}

bool XboxPurchaser::IsSuccess() const {
    MILO_ASSERT(!IsPurchasing(), 0x3c3);
    return mState == kSuccess;
}

bool XboxPurchaser::PurchaseMade() const {
    MILO_ASSERT(mState == kSuccess, 0x3c9);
    return false;
}

bool XboxPurchaser::IsPurchasing() const {
    return !(mState == kIdle || mState == kSuccess || mState == kFailure);
}

DataNode XboxPurchaser::OnMsg(UIChangedMsg const &msg) {
    if (mState != kPurchasing || msg.Showing()) {
        return 0;
    } else {
        static Symbol ui_changed("ui_changed");
        ThePlatformMgr.RemoveSink(this, ui_changed);
        mState = kSuccess;
        return 0;
    }
}

BEGIN_HANDLERS(XboxPurchaser)
    HANDLE_MESSAGE(UIChangedMsg)
END_HANDLERS

#pragma endregion XboxPurchaser
#pragma region XboxMultipleItemsPurchaser

bool XboxMultipleItemsPurchaser::IsSuccess() const {
    MILO_ASSERT(!IsPurchasing(), 0x365);
    return mState == kSuccess;
}

bool XboxMultipleItemsPurchaser::PurchaseMade() const {
    MILO_ASSERT(mState == kSuccess, 0x36b);
    return false;
}

bool XboxMultipleItemsPurchaser::IsPurchasing() const {
    return !(mState == kIdle || mState == kSuccess || mState == kFailure);
}

void XboxMultipleItemsPurchaser::Initiate() {
    MILO_ASSERT(!IsPurchasing(), 0x343);
    mState = kPurchasing;

    memset(&sOverlapped, 0, sizeof(_XOVERLAPPED));
    mResult = 0;
    DWORD result = XShowMarketplaceDownloadItemsUI(
        mPadNum, 0x3e9, &mOfferIDs[0], mOfferIDs.size(), &mResult, &sOverlapped
    );
    if (result != 0x3e5) {
        MILO_NOTIFY("Error starting checkout UI: %d", result);
        mState = kFailure;
    }
    static Symbol ui_changed("ui_changed");
    ThePlatformMgr.AddSink(this, ui_changed);
}

XboxMultipleItemsPurchaser::~XboxMultipleItemsPurchaser() {
    static Symbol ui_changed("ui_changed");
    ThePlatformMgr.RemoveSink(this, ui_changed);
}

XboxMultipleItemsPurchaser::XboxMultipleItemsPurchaser(
    int i, std::vector<unsigned long long> &offerIDs, Symbol s, unsigned int ui
)
    : StorePurchaser(s, ui), mState(kIdle), mPadNum(i) {
    MILO_ASSERT(offerIDs.size() >= 1 && offerIDs.size() <= XMARKETPLACE_MAX_OFFERIDS, 0x337);
    mOfferIDs = offerIDs;
}

DataNode XboxMultipleItemsPurchaser::OnMsg(UIChangedMsg const &msg) {
    if (mState != kPurchasing || msg.Showing()) {
        return 0;
    } else {
        static Symbol ui_changed("ui_changed");
        ThePlatformMgr.RemoveSink(this, ui_changed);
        mState = kSuccess;
        return 0;
    }
}

BEGIN_HANDLERS(XboxMultipleItemsPurchaser)
    HANDLE_MESSAGE(UIChangedMsg)
END_HANDLERS

#pragma endregion XboxMultipleItemsPurchaser
