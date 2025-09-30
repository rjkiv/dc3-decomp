#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "utl/Symbol.h"

class StorePurchaser {
public:
    ~StorePurchaser();
};

class XboxPurchaser : public Hmx::Object, public StorePurchaser {
public:
    // Hmx::Object
    ~XboxPurchaser();
    virtual DataNode Handle(DataArray *, bool);

    // StorePurchaser
    virtual void Initiate();
    virtual bool IsPurchasing() const;
    virtual bool IsSuccess() const;
    virtual bool PurchaseMade() const;

    XboxPurchaser(int, u64, u64, u64, Symbol, unsigned int);

    Symbol unk4;
    unsigned int unk8;
    Hmx::Object unkc;
    int unk38;
    u64 unk40;
    int unk48;

private:
    DataNode OnMsg(UIChangedMsg const &);
};

class XboxMultipleItemsPurchaser : public Hmx::Object, public StorePurchaser {
public:
    // Hmx::Object
    virtual ~XboxMultipleItemsPurchaser();
    virtual DataNode Handle(DataArray *, bool);

    // StorePurchaser
    virtual void Initiate();
    virtual bool IsPurchasing() const;
    virtual bool IsSuccess() const;
    virtual bool PurchaseMade() const;

    XboxMultipleItemsPurchaser(int, std::vector<u64>, Symbol, unsigned int);

private:
    DataNode OnMsg(UIChangedMsg const &);
};
