#include "net_ham/TokenJobs.h"
#include "RCJobDingo.h"
#include "net/JsonUtils.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

RedeemTokenJob::RedeemTokenJob(Hmx::Object *callback, int, String str)
    : RCJob("trs/redeemtoken/", callback) {
    DataPoint dataP;
    static Symbol token("token");
    dataP.AddPair(token, str);
    SetDataPoint(dataP);
}

bool RedeemTokenJob::GetRedeemTokenData(int &iref, String &sref) {
    iref = mResult;
    JsonConverter &reader = mJsonReader;
    if (!mJsonResponse) {
        return false;
    } else {
        JsonObject *obj = reader.GetByName(mJsonResponse, "offers");
        if (obj) {
            if (obj->GetType() == JsonObject::kType_String) {
                sref = obj->Str();
            } else if (obj->GetType() == JsonObject::kType_Array) {
                sref = reader.Str(static_cast<JsonArray *>(obj), 0);
            }
        }
        return true;
    }
}
