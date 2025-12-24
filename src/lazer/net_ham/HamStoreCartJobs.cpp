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

CartRow::CartRow() {}

CartRow::CartRow(CartRow const &cr) : unk0(cr.unk0), unk4(cr.unk4), unkc(cr.unkc) {}

LockCartJob::LockCartJob(Hmx::Object *o, char const *c) : RCJob("dlc/lockcart/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, DataNode(c));
    SetDataPoint(dataP);
}

void LockCartJob::GetLockData(int &reLockDuration) {
    if (mResult == 1 && mJsonResponse) {
        JsonObject *pJsonObj = mJsonConverter.GetByName(mJsonResponse, "lock_duration");
        if (pJsonObj) {
            reLockDuration = (pJsonObj->Int() - 60) * 1000;
        }
        MILO_ASSERT(reLockDuration > 0, 0x8a);
    }
}

UnlockCartJob::UnlockCartJob(Hmx::Object *o, char const *onlineID)
    : RCJob("dlc/unlockcart/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, DataNode(onlineID));
    SetDataPoint(dataP);
}

AddDLCToCartJob::AddDLCToCartJob(Hmx::Object *o, char const *onlineID, int songID)
    : RCJob("dlc/adddlctocart/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol song_id("song_id");
    dataP.AddPair(pid, DataNode(onlineID));
    dataP.AddPair(song_id, DataNode(songID));
    SetDataPoint(dataP);
}

RemoveDLCFromCartJob::RemoveDLCFromCartJob(
    Hmx::Object *o, char const *onlineID, int songID
)
    : RCJob("dlc/removedlcfromcart/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol song_id("song_id");
    dataP.AddPair(pid, DataNode(onlineID));
    dataP.AddPair(song_id, DataNode(songID));
    SetDataPoint(dataP);
}

EmptyCartJob::EmptyCartJob(Hmx::Object *o, char const *onlineID)
    : RCJob("dlc/emptycart/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, DataNode(onlineID));
    SetDataPoint(dataP);
}

GetCartJob::GetCartJob(Hmx::Object *o, HamProfile *hp) : RCJob("dlc/getcart/", o) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, DataNode(hp->GetOnlineID()->ToString()));
    SetDataPoint(dataP);
}

void GetCartJob::GetRows(std::vector<CartRow> *rows) {
    if (mResult != 1)
        return;

    if (!mJsonResponse)
        return;

    ::GetRows(mJsonConverter, mJsonResponse, rows);
}
