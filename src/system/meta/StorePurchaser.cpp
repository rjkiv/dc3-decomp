#include "meta/StorePurchaser.h"
#include "meta/StoreOffer.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "ui/UI.h"
#include "utl/Symbol.h"

#pragma region XboxPurchaser

XboxPurchaser::XboxPurchaser(
    int param1,
    unsigned long long param2,
    unsigned long long param3,
    unsigned long long param4,
    Symbol s,
    unsigned int ui
)
    : StorePurchaser(s, ui), mState(state0), unk40(param2), unk48(param1) {}

XboxPurchaser::~XboxPurchaser() {
    static Symbol ui_changed("ui_changed");
    ThePlatformMgr.RemoveSink(this, ui_changed);
}

void XboxPurchaser::Initiate() {
    MILO_ASSERT(!IsPurchasing(), 0x39a);

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
    return !(mState == state0 || mState == kSuccess || mState == state3);
}

DataNode XboxPurchaser::OnMsg(UIChangedMsg const &) { return NULL_OBJ; }

BEGIN_HANDLERS(XboxPurchaser)
    HANDLE_SUPERCLASS(Hmx::Object)
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
    return !(mState == state0 || mState == kSuccess || mState == state3);
}

void XboxMultipleItemsPurchaser::Initiate() { MILO_ASSERT(!IsPurchasing(), 0x343); }

XboxMultipleItemsPurchaser::~XboxMultipleItemsPurchaser() {
    static Symbol ui_changed("ui_changed");
    ThePlatformMgr.RemoveSink(this, ui_changed);
}

XboxMultipleItemsPurchaser::XboxMultipleItemsPurchaser(
    int i, std::vector<unsigned long long> &offerIDs, Symbol s, unsigned int ui
)
    : StorePurchaser(s, ui), mState(state0), unk48(i) {
    MILO_ASSERT(offerIDs.size() >= 1 && offerIDs.size() <= 6, 0x337);
    unk3c = offerIDs;
}

DataNode XboxMultipleItemsPurchaser::OnMsg(UIChangedMsg const &) { return NULL_OBJ; }

BEGIN_HANDLERS(XboxMultipleItemsPurchaser)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

#pragma endregion XboxMultipleItemsPurchaser
