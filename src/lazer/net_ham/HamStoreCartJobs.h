#pragma once
#include "meta_ham/HamProfile.h"
#include "net/JsonUtils.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "utl/Str.h"

class CartRow {
public:
    CartRow();
    CartRow(CartRow const &);

    const CartRow *unk0;
    String unk4;
    String unkc;
};

class LockCartJob : public RCJob {
public:
    LockCartJob(Hmx::Object *, char const *);
    void GetLockData(int &);
};

class UnlockCartJob : public RCJob {
public:
    UnlockCartJob(Hmx::Object *, char const *);
};

class AddDLCToCartJob : public RCJob {
public:
    AddDLCToCartJob(Hmx::Object *, char const *, int);
};

class RemoveDLCFromCartJob : public RCJob {
public:
    RemoveDLCFromCartJob(Hmx::Object *, char const *, int);
};

class EmptyCartJob : public RCJob {
public:
    EmptyCartJob(Hmx::Object *, char const *);
};

class GetCartJob : public RCJob {
public:
    GetCartJob(Hmx::Object *, HamProfile *);
    void GetRows(std::vector<CartRow> *);
};

void GetRows(JsonConverter &, JsonObject const *, std::vector<CartRow> *);
