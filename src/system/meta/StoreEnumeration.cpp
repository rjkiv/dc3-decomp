#include "meta/StoreEnumeration.h"
#include "macros.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "xdk/win_types.h"
#include "xdk/XAPILIB.h"
#include "xdk/xonline/xonline.h"
#include <cstring>

XboxEnumeration::XboxEnumeration(int i, std::vector<QWORD> *offerIDs)
    : mOfferIDCount(0), unk10(nullptr), unk14(nullptr), unk18(i), mSuccess(false),
      mHandle(0), mBufferSizeBytes(0), mCurOffers(0) {
    if (offerIDs) {
        mOfferIDCount = offerIDs->size();
        MILO_ASSERT(mOfferIDCount, 0x197);
        unk10 = new QWORD[mOfferIDCount];
        memcpy(unk10, offerIDs->begin(), mOfferIDCount * sizeof(QWORD));
        unk14 = unk10;
    }
}

XboxEnumeration::~XboxEnumeration() {
    RELEASE(unk10);
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
        mBufferSizeBytes = 0;
        if (unk14 == unk10) {
            mContentList.clear();
        }
        DWORD result;
        if (!unk10) {
            result = XMarketplaceCreateOfferEnumerator(
                unk18, 0x100002, -1, 99, &mBufferSizeBytes, &mHandle
            );
        } else {
            int val = mOfferIDCount - (unk14 - unk10);
            if (99 <= val) {
                val = 99;
            }
            result = XMarketplaceCreateOfferEnumeratorByOffering(
                unk18, val, unk14, val, &mBufferSizeBytes, &mHandle
            );
            unk14 += val;
        }
        MILO_ASSERT(!mCurOffers, 0x1ea);
        // mCurOffers = new HANDLE[1]; // needs to be a HANDLE[], but the total size in
        // bytes is mBufferSizeBytes

        mCurOffers = (HANDLE *)operator new[](mBufferSizeBytes);
        if (result != ERROR_SUCCESS) {
            goto thing;
        }
    }
    memset(mCurOffers, 0, mBufferSizeBytes);
    memset(&mOverlapped, 0, sizeof(XOVERLAPPED));

    DWORD enumerate = XEnumerate(mHandle, mCurOffers, mBufferSizeBytes, 0, &mOverlapped);
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

        mContentList.push_back(product);
    }

    // lVar1 = (longlong)(int)this;
    // if (*(int *)(this + 0x3c) == 0) {
    //   return;
    // }
    // lVar4 = lVar1 + 0x20;
    // if (*(int *)(this + 0x20) == 0x3e5) {
    //   return;
    // }
    // local_1198 = 0;
    // pvVar2 = (void *)XGetOverlappedResult(lVar4,&local_1198,0);
    // uVar5 = 0;
    // local_1194 = pvVar2;
    // if (local_1198 != 0) {
    //   iVar6 = 0;
    //   local_11a0 = (uint)(lVar1 + 4);
    //   do {
    //     String::String(&SStack_1190);
    //     puVar7 = (undefined8 *)(iVar6 + *(int *)(this + 0x44));
    //     WideCharToMultiByte(0,0,*(LPCWSTR *)((int)puVar7 + 0x14),*(int *)(puVar7 +
    //     2),aCStack_1170,
    //                         0xff,(LPCSTR)0x0,(LPBOOL)0x0);
    //     String::operator=(&SStack_1190,aCStack_1170);
    //     local_1188 = *puVar7;
    //     local_1180 = *(undefined4 *)(puVar7 + 9);
    //     local_117c = *(undefined4 *)((int)puVar7 + 100);
    //     stlpmtx_std::list<>::insert((list<> *)&local_119c,lVar1 +
    //     4,&local_11a0,&SStack_1190); String::~String(&SStack_1190); uVar5 = uVar5 + 1;
    //     iVar6 = iVar6 + 0x68;
    //   } while (uVar5 < local_1198);
    // }
}
