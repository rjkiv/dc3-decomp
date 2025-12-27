#include "net_ham/MotdJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

GetMotdJob::GetMotdJob(Hmx::Object *callback) : RCJob("motd/getmotd/", callback) {
    DataPoint dataP;
    static Symbol locale("locale");
    dataP.AddPair(locale, SystemLanguage());
    SetDataPoint(dataP);
}
