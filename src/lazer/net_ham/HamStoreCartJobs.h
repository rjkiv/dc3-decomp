#pragma once
#include "meta_ham/HamProfile.h"
#include "net/JsonUtils.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "utl/Str.h"

class CartRow {
public:
    int unk0;
    String unk4;
    String unkc;
};

class LockCartJob : public RCJob {
public:
    LockCartJob(Hmx::Object *callback, const char *onlineID);
    void GetLockData(int &);
};

class UnlockCartJob : public RCJob {
public:
    UnlockCartJob(Hmx::Object *callback, const char *onlineID);
};

class AddDLCToCartJob : public RCJob {
public:
    AddDLCToCartJob(Hmx::Object *callback, const char *onlineID, int songID);
};

class RemoveDLCFromCartJob : public RCJob {
public:
    RemoveDLCFromCartJob(Hmx::Object *callback, const char *onlineID, int songID);
};

class EmptyCartJob : public RCJob {
public:
    EmptyCartJob(Hmx::Object *callback, const char *onlineID);
};

class GetCartJob : public RCJob {
public:
    GetCartJob(Hmx::Object *callback, HamProfile *);
    void GetRows(std::vector<CartRow> *);
};
