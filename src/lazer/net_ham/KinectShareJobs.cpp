#include "net_ham/KinectShareJobs.h"
#include "RCJobDingo.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"

KinectShareJob::KinectShareJob(Hmx::Object *callback)
    : RCJob("motd/kinectshareupload/", callback) {
    DataPoint dataP;
    SetDataPoint(dataP);
}
