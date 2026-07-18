#pragma once
#include "meta/StoreEnumeration.h"
#include "types.h"
#include "utl/Str.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"
#include <list>

enum StoreError {
    kStoreErrorSuccess = 0,
    kStoreErrorNoContent = 1,
    kStoreErrorCacheNoSpace = 2,
    kStoreErrorCacheRemoved = 3,
    kStoreErrorLiveServer = 4,
    kStoreErrorStoreServer = 5,
    kStoreErrorSignedOut = 6,
    kStoreErrorNoMetadata = 7,
    kStoreErrorEcommerce = 8,
    kStoreErrorNoEula = 9
};

struct EnumProduct {
    String unk0;
    QWORD unk8;
    int unk10;
    int unk14;
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

protected:
    std::list<EnumProduct> mContentList; // 0x4
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

private:
    int mOfferIDCount; // 0xc
    QWORD *unk10; // 0x10 - offer IDs
    QWORD *unk14; // 0x14 - offer ID iterator?
    int unk18; // 0x18 - padnum?
    bool mSuccess; // 0x1c
    XOVERLAPPED mOverlapped; // 0x20
    HANDLE mHandle; // 0x3c
    DWORD mBufferSizeBytes; // 0x40
    HANDLE *mCurOffers; // 0x44 - array of structs of size 0x68 each
};
