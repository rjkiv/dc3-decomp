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
    : StorePurchaser(s, ui), mState(purchasestate0), unk40(param2), unk48(param1) {}

XboxPurchaser::~XboxPurchaser() {
    static Symbol ui_changed("ui_changed");
    ThePlatformMgr.RemoveSink(this, ui_changed);
}

void XboxPurchaser::Initiate() {
    MILO_ASSERT(!IsPurchasing(), 0x39a);
    mState = purchasestate1;
    DWORD result;
    DWORD id;
    if (PlatformMgr::sXShowCallback(id)) {
        result = XShowNuiMarketplaceUI(id, unk48, 5, unk40, -1);
    } else {
        result = XShowMarketplaceUI(unk48, 5, unk40, -1);
    }

    if (result != 0) {
        MILO_NOTIFY("Error starting checkout UI: %d", result);
        mState = purchasestate3;
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
    return !(mState == purchasestate0 || mState == kSuccess || mState == purchasestate3);
}

DataNode XboxPurchaser::OnMsg(UIChangedMsg const &msg) {
    if (mState != purchasestate1 || msg.Showing()) {
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
    return !(mState == purchasestate0 || mState == kSuccess || mState == purchasestate3);
}

void XboxMultipleItemsPurchaser::Initiate() {
    MILO_ASSERT(!IsPurchasing(), 0x343);
    mState = purchasestate1;

    memset(&sOverlapped, 0, sizeof(_XOVERLAPPED));
    unk4c = 0;
    DWORD result = XShowMarketplaceDownloadItemsUI(
        unk48, 0x3e9, &unk3c[0], unk3c.size(), &unk4c, &sOverlapped
    );
    if (result != 0x3e5) {
        MILO_NOTIFY("Error starting checkout UI: %d", result);
        mState = purchasestate3;
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
    : StorePurchaser(s, ui), mState(purchasestate0), unk48(i) {
    MILO_ASSERT(offerIDs.size() >= 1 && offerIDs.size() <= XMARKETPLACE_MAX_OFFERIDS, 0x337);
    unk3c = offerIDs;
}

DataNode XboxMultipleItemsPurchaser::OnMsg(UIChangedMsg const &msg) {
    if (mState != purchasestate1 || msg.Showing()) {
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
