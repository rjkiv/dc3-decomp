#include "net_ham/WebLinkJobs.h"
#include "RCJobDingo.h"
#include "WebLinkJobs.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"
#include "utl/Str.h"

GetWebLinkCodeJob::GetWebLinkCodeJob(Hmx::Object *o, int i)
    : RCJob("auth/getwebsitepasscode/", o) {
    DataPoint dataP;
    SetDataPoint(dataP);
}

bool GetWebLinkCodeJob::GetWebLinkCodeData(String &str) {
    if (mResult != 1)
        return false;

    if (!mJsonResponse)
        return false;

    str = mJsonResponse->Str();
    return true;
}
