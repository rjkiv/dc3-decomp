#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UI.h"
#include "utl/Symbol.h"

class StorePurchaser{
    public:
        ~StorePurchaser();
};

class XboxPurchaser : public Hmx::Object, public StorePurchaser{
    public:
        XboxPurchaser(int, u64, u64, u64, Symbol, unsigned int);
        ~XboxPurchaser();
        virtual void Initiate();
        virtual bool IsSuccess() const;
        virtual bool PurchaseMade() const;
        virtual bool IsPurchasing() const;
        virtual DataNode Handle(DataArray *, bool);

        
        int unk38;
        u64 unk40;
        int unk48;
    private:
        DataNode OnMsg(UIChangedMsg const &);
};

class XboxMultipleItemsPurchaser : public StorePurchaser, Hmx::Object{
    public:
        virtual bool IsSuccess() const;
        virtual bool PurchaseMade() const;
        virtual bool IsPurchasing() const;
        virtual void Initiate();
        virtual DataNode Handle(DataArray *, bool);
        virtual ~XboxMultipleItemsPurchaser();
        XboxMultipleItemsPurchaser(int, std::vector<u64>, Symbol, unsigned int);
    private:
        DataNode OnMsg(UIChangedMsg const &);
};
