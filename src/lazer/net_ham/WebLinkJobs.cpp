#include "net_ham/WebLinkJobs.h"
#include "RCJobDingo.h"
#include "WebLinkJobs.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"
#include "utl/Str.h"

GetWebLinkCodeJob::GetWebLinkCodeJob(Hmx::Object *callback, int)
    : RCJob("auth/getwebsitepasscode/", callback) {
    DataPoint dataP;
    SetDataPoint(dataP);
}

bool GetWebLinkCodeJob::GetWebLinkCodeData(String &str) {
    if (mResult != 1) {
        return false;
    } else if (!mJsonResponse) {
        return false;
    } else {
        str = mJsonResponse->Str();
        return true;
    }
}
