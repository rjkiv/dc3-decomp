#include "net_ham/TokenJobs.h"
#include "RCJobDingo.h"
#include "net/JsonUtils.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

RedeemTokenJob::RedeemTokenJob(Hmx::Object *o, int i, String str)
    : RCJob("trs/redeemtoken/", o) {
    DataPoint dataP;
    static Symbol token("token");
    dataP.AddPair(token, DataNode(str));
    SetDataPoint(dataP);
}
