#include "net_ham/HamStoreCartJobs.h"
#include "meta_ham/HamProfile.h"
#include "net/JsonUtils.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "stl/_vector.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

LockCartJob::LockCartJob(Hmx::Object *callback, const char *onlineID)
    : RCJob("dlc/lockcart/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, onlineID);
    SetDataPoint(dataP);
}

void LockCartJob::GetLockData(int &reLockDuration) {
    if (mResult == 1 && mJsonResponse) {
        JsonObject *pJsonObj = mJsonReader.GetByName(mJsonResponse, "lock_duration");
        if (pJsonObj) {
            reLockDuration = (pJsonObj->Int() - 60) * 1000;
        }
        MILO_ASSERT(reLockDuration > 0, 0x8a);
    }
}

UnlockCartJob::UnlockCartJob(Hmx::Object *callback, const char *onlineID)
    : RCJob("dlc/unlockcart/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, onlineID);
    SetDataPoint(dataP);
}

AddDLCToCartJob::AddDLCToCartJob(Hmx::Object *callback, const char *onlineID, int songID)
    : RCJob("dlc/adddlctocart/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol song_id("song_id");
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(song_id, songID);
    SetDataPoint(dataP);
}

RemoveDLCFromCartJob::RemoveDLCFromCartJob(
    Hmx::Object *callback, const char *onlineID, int songID
)
    : RCJob("dlc/removedlcfromcart/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol song_id("song_id");
    dataP.AddPair(pid, onlineID);
    dataP.AddPair(song_id, songID);
    SetDataPoint(dataP);
}

EmptyCartJob::EmptyCartJob(Hmx::Object *callback, const char *onlineID)
    : RCJob("dlc/emptycart/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, onlineID);
    SetDataPoint(dataP);
}

GetCartJob::GetCartJob(Hmx::Object *callback, HamProfile *hp)
    : RCJob("dlc/getcart/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, hp->GetOnlineID()->ToString());
    SetDataPoint(dataP);
}

void GetRows(JsonConverter &c, const JsonObject *o, std::vector<CartRow> *rows) {
    JsonArray *a = const_cast<JsonArray *>(static_cast<const JsonArray *>(o));
    unsigned int aSize = a->GetSize();
    for (int i = 0; i < aSize; i++) {
        JsonArray *cur = static_cast<JsonArray *>(c.GetValue(a, i));
        CartRow row;
        row.unk0 = c.GetValue(cur, 0)->Int();
        row.unk4 = c.GetValue(cur, 1)->Str();
        row.unkc = c.GetValue(cur, 2)->Str();
        rows->push_back(row);
    }
}

void GetCartJob::GetRows(std::vector<CartRow> *rows) {
    if (mResult == 1 && mJsonResponse) {
        ::GetRows(mJsonReader, mJsonResponse, rows);
    }
}
