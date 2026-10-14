#pragma once
#include "meta/StoreEnumeration.h"
#include "types.h"
#include "utl/Str.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xonline/xonline.h"
#include <list>

enum StoreError {
    kStoreErrorSuccess = 0x0000,
    kStoreErrorNoContent = 0x0001,
    kStoreErrorLiveServer = 0x0002,
    kStoreErrorStoreServer = 0x0003,
    kStoreErrorSignedOut = 0x0004,
    kStoreErrorDroppedOut = 0x0005,
    kStoreErrorNoMetadata = 0x0006,
    kStoreErrorNoEthernetCable = 0x0007
};

struct EnumProduct {
    String mName; // 0x0
    QWORD mOfferID; // 0x8
    int mPurchased; // 0x10
    int mPrice; // 0x14
};

class StoreEnumeration {
public:
    enum State {
        kEnumWaiting = 0,
        kEnumProcessing = 1,
        kPreSuccess = 2,
        kPreFail = 3,
        kSuccess = 4,
        kFail = 5,
    };
    StoreEnumeration() {}
    virtual ~StoreEnumeration() {}
    virtual void Start() = 0;
    virtual bool IsEnumerating() const = 0;
    virtual bool IsSuccess() const = 0;
    virtual void Poll() = 0;

    std::list<EnumProduct> mProducts; // 0x4
};

class XboxEnumeration : public StoreEnumeration {
public:
    // StoreEnumeration
    virtual ~XboxEnumeration();
    virtual void Start();
    virtual bool IsEnumerating() const;
    virtual bool IsSuccess() const;
    virtual void Poll();

    XboxEnumeration(int, std::vector<QWORD> *);

protected:
    int mOfferIDCount; // 0xc
    QWORD *mOfferIDs; // 0x10
    QWORD *mNextOfferID; // 0x14
    int mPadNum; // 0x18 - padnum?
    bool mSuccess; // 0x1c

private:
    XOVERLAPPED mOverlapped; // 0x20
    HANDLE mHandle; // 0x3c
    DWORD mBufSizeBytes; // 0x40
    XMARKETPLACE_CONTENTOFFER_INFO *mCurOffers; // 0x44
};
