#include "meta/StoreEnumeration.h"
#include "macros.h"
#include "os/Debug.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/xbox.h"
#include "xdk/xonline/xonline.h"
#include <cstring>

EnumProduct::EnumProduct(EnumProduct const &product)
    : unk0(product.unk0), unk8(product.unk8), unk10(product.unk10), unk14(product.unk14) {
}

XboxEnumeration::XboxEnumeration(int i, std::vector<unsigned long long> *offerIDs)
    : mOfferIDCount(0), unk10(nullptr), unk14(nullptr), unk18(i), unk1c(false),
      mHandle(0), unk40(0), mCurOffers(0) {
    if (offerIDs) {
        mOfferIDCount = offerIDs->size();
        MILO_ASSERT(mOfferIDCount, 0x197);
        int arrSize = mOfferIDCount << 3;
        unk10 = new int[arrSize];
        memcpy(unk10, &offerIDs[0], mOfferIDCount << 3);
        unk14 = unk10;
    }
}

XboxEnumeration::~XboxEnumeration() {
    RELEASE(unk10);
    if (mHandle && unk20.InternalLow == 0x3e5) {
        DWORD result = XCancelOverlapped(&unk20);
        if (result != 0) {
            MILO_FAIL("Error cancelling enum %d", result);
        }
    }

    if (mHandle) {
        CloseHandle(mHandle);
        mHandle = 0;
    }
    RELEASE(mCurOffers);
}

bool XboxEnumeration::IsSuccess() const {
    MILO_ASSERT(!mHandle, 0x208);
    return unk1c;
}

void XboxEnumeration::Start() {
    unk1c = true;
    if (!mHandle) {
        unk40 = 0;
        if (unk14 == unk10) {
            mContentList.clear();
        }
        int result;
        if (!unk10) {
            result = XMarketplaceCreateOfferEnumerator(
                unk18, 0x100002, -1, 99, &unk40, &mCurOffers
            );
        } else {
        }
        MILO_ASSERT(!mCurOffers, 0x1ea);
    }
}

void XboxEnumeration::Poll() {}
