#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "utl/Symbol.h"

enum PurchaseState { // just know the val of kSuccess
    purchasestate0 = 0,
    purchasestate1 = 1,
    kSuccess = 2,
    purchasestate3 = 3,
};

class StorePurchaser {
public:
    virtual ~StorePurchaser() {}

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
    int unk48;

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

    XboxMultipleItemsPurchaser(
        int, std::vector<unsigned long long> &, Symbol, unsigned int
    );

    PurchaseState mState; // 0x38
    std::vector<unsigned long long> unk3c;
    int unk48;

private:
    DataNode OnMsg(UIChangedMsg const &);
};
