#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UI.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"

const int XMARKETPLACE_MAX_OFFERIDS = 6;

enum PurchaseState { // just know the val of kSuccess
    purchasestate0 = 0,
    purchasestate1 = 1,
    kSuccess = 2,
    purchasestate3 = 3,
};

class StorePurchaser {
public:
    virtual ~StorePurchaser() {}
    virtual void Initiate() = 0; // 0x4
    virtual bool IsPurchasing() const = 0; // 0x8
    virtual bool IsSuccess() const = 0; // 0xc
    virtual bool PurchaseMade() const = 0; // 0x10
    virtual bool NeedsEnum() const { return false; } // 0x14
    virtual void Poll() = 0; // 0x18

    StorePurchaser(Symbol s, unsigned int i) : unk4(s), unk8(i) {}

    Symbol unk4;
    unsigned int unk8;
};

class XboxPurchaser : public StorePurchaser, public Hmx::Object {
public:
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

    int GetUnk48() const { return unk48; }

    PurchaseState mState; // 0x38
    u32 unk3c;
    unsigned long long unk40;
    DWORD unk48;

private:
    DataNode OnMsg(UIChangedMsg const &);
};

class XboxMultipleItemsPurchaser : public StorePurchaser, Hmx::Object {
public:
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

    PurchaseState mState; // 0x38
    std::vector<unsigned long long> unk3c;
    DWORD unk48;
    HRESULT unk4c;

private:
    static _XOVERLAPPED sOverlapped;

    DataNode OnMsg(UIChangedMsg const &);
};
