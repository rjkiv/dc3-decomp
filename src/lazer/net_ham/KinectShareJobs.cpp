#include "net_ham/KinectShareJobs.h"
#include "RCJobDingo.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"

KinectShareJob::KinectShareJob(Hmx::Object *o) : RCJob("motd/kinectshareupload/", o) {
    DataPoint dataP;
    SetDataPoint(dataP);
}
