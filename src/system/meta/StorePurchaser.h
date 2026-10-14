#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UI.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"

const int XMARKETPLACE_MAX_OFFERIDS = 6;

class StorePurchaser {
public:
    virtual ~StorePurchaser() {}
    virtual void Initiate() = 0; // 0x4
    virtual bool IsPurchasing() const = 0; // 0x8
    virtual bool IsSuccess() const = 0; // 0xc
    virtual bool PurchaseMade() const = 0; // 0x10
    virtual bool NeedsEnum() const { return false; } // 0x14
    virtual void Poll() = 0; // 0x18

    StorePurchaser(Symbol s, unsigned int i) : mSource(s), mPID(i) {}

    Symbol Source() const { return mSource; }
    unsigned int PID() const { return mPID; }

protected:
    Symbol mSource; // 0x4
    unsigned int mPID; // 0x8
};

class XboxPurchaser : public StorePurchaser, public Hmx::Object {
public:
    enum State {
        kIdle = 0x0000,
        kPurchasing = 0x0001,
        kSuccess = 0x0002,
        kFailure = 0x0003
    };

    // Hmx::Object
    virtual ~XboxPurchaser();
    virtual DataNode Handle(DataArray *, bool);

    // StorePurchaser
    virtual void Initiate(); // 0x4
    virtual bool IsPurchasing() const; // 0x8
    virtual bool IsSuccess() const; // 0xc
    virtual bool PurchaseMade() const; // 0x10
    virtual bool NeedsEnum() const { return true; }; // 0x14
    virtual void Poll() {}; // 0x18

    XboxPurchaser(
        int,
        unsigned long long,
        unsigned long long,
        unsigned long long,
        Symbol,
        unsigned int
    );

private:
    DataNode OnMsg(UIChangedMsg const &);

    State mState; // 0x38
    unsigned long long mOfferID; // 0x40
    int mPadNum; // 0x48
};

class XboxMultipleItemsPurchaser : public StorePurchaser, Hmx::Object {
public:
    enum State {
        kIdle = 0x0000,
        kPurchasing = 0x0001,
        kSuccess = 0x0002,
        kFailure = 0x0003
    };

    // Hmx::Object
    virtual ~XboxMultipleItemsPurchaser();
    virtual DataNode Handle(DataArray *, bool);

    // StorePurchaser
    virtual void Initiate();
    virtual bool IsPurchasing() const;
    virtual bool IsSuccess() const;
    virtual bool PurchaseMade() const;
    virtual bool NeedsEnum() const { return true; }
    virtual void Poll() {}

    XboxMultipleItemsPurchaser(
        int, std::vector<unsigned long long> &, Symbol, unsigned int
    );

private:
    DataNode OnMsg(UIChangedMsg const &);

    static _XOVERLAPPED sOverlapped;

    State mState; // 0x38
    std::vector<unsigned long long> mOfferIDs; // 0x3c
    int mPadNum; // 0x48
    HRESULT mResult; // 0x4c - int in pdb
};
