#include "meta/StoreEnumeration.h"
#include "macros.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "xdk/win_types.h"
#include "xdk/XAPILIB.h"
#include "xdk/xonline/xonline.h"
#include <cstring>

XboxEnumeration::XboxEnumeration(int i, std::vector<QWORD> *offerIDs)
    : mOfferIDCount(0), mOfferIDs(nullptr), mNextOfferID(nullptr), mPadNum(i),
      mSuccess(false), mHandle(0), mBufSizeBytes(0), mCurOffers(0) {
    if (offerIDs) {
        mOfferIDCount = offerIDs->size();
        MILO_ASSERT(mOfferIDCount, 0x197);
        mOfferIDs = new QWORD[mOfferIDCount];
        memcpy(mOfferIDs, offerIDs->begin(), mOfferIDCount * sizeof(QWORD));
        mNextOfferID = mOfferIDs;
    }
}

XboxEnumeration::~XboxEnumeration() {
    RELEASE(mOfferIDs);
    if (mHandle && mOverlapped.InternalLow == ERROR_IO_PENDING) {
        DWORD result = XCancelOverlapped(&mOverlapped);
        if (result != ERROR_SUCCESS) {
            MILO_FAIL("Error cancelling enum %d", result);
        }
    }

    if (mHandle) {
        CloseHandle(mHandle);
        mHandle = nullptr;
    }
    RELEASE(mCurOffers);
}

bool XboxEnumeration::IsSuccess() const {
    MILO_ASSERT(!mHandle, 0x208);
    return mSuccess;
}

void XboxEnumeration::Start() {
    mSuccess = true;
    if (!mHandle) {
        mBufSizeBytes = 0;
        if (mNextOfferID == mOfferIDs) {
            mProducts.clear();
        }
        DWORD result;
        if (!mOfferIDs) {
            result = XMarketplaceCreateOfferEnumerator(
                mPadNum, 0x100002, -1, 99, &mBufSizeBytes, &mHandle
            );
        } else {
            int val = mOfferIDCount - (mNextOfferID - mOfferIDs);
            if (99 <= val) {
                val = 99;
            }
            result = XMarketplaceCreateOfferEnumeratorByOffering(
                mPadNum, val, mNextOfferID, val, &mBufSizeBytes, &mHandle
            );
            mNextOfferID += val;
        }
        MILO_ASSERT(!mCurOffers, 0x1ea);
        // mCurOffers = new HANDLE[1]; // needs to be a HANDLE[], but the total size in
        // bytes is mBufSizeBytes

        mCurOffers = (XMARKETPLACE_CONTENTOFFER_INFO *)operator new[](mBufSizeBytes);
        if (result != ERROR_SUCCESS) {
            goto thing;
        }
    }
    memset(mCurOffers, 0, mBufSizeBytes);
    memset(&mOverlapped, 0, sizeof(XOVERLAPPED));

    DWORD enumerate = XEnumerate(mHandle, mCurOffers, mBufSizeBytes, 0, &mOverlapped);
    if (enumerate == ERROR_IO_PENDING) {
        return;
    }
thing:
    if (mHandle) {
        CloseHandle(mHandle);
        mHandle = 0;
    }
    delete mCurOffers;
    mSuccess = false;
    mCurOffers = nullptr;
}

void XboxEnumeration::Poll() {
    if (!mHandle) {
        return;
    }
    if (mOverlapped.InternalLow == ERROR_IO_PENDING) {
        return;
    }
    DWORD dwResult = 0;
    DWORD res = XGetOverlappedResult(&mOverlapped, &dwResult, false);
    for (int i = 0; i < dwResult; i++) {
        EnumProduct product;
        // WideCharToMultiByte(0, 0, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR
        // lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL
        // lpUsedDefaultChar)

        mProducts.push_back(product);
    }
}
