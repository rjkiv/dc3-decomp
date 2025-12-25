#include "net_ham/FitnessGoalJobs.h"
#include "meta_ham/HamProfile.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

GetFitnessGoalJob::GetFitnessGoalJob(Hmx::Object *callback, char const *onlineID)
    : RCJob("fitness/getfitnessgoal/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    dataP.AddPair(pid, onlineID);
    SetDataPoint(dataP);
}

SetFitnessGoalJob::SetFitnessGoalJob(Hmx::Object *callback, HamProfile *hp)
    : RCJob("fitness/setfitnessgoal/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol days_active("days_active");
    static Symbol calories("calories");
    int daysActive = 0;
    int cals = 0;
    hp->GetFitnessGoal(daysActive, cals);
    dataP.AddPair(pid, hp->GetOnlineID()->ToString());
    dataP.AddPair(days_active, daysActive);
    dataP.AddPair(calories, cals);
    SetDataPoint(dataP);
}

DeleteFitnessGoalJob::DeleteFitnessGoalJob(Hmx::Object *callback, HamProfile *hp)
    : RCJob("fitness/deletefitnessgoal/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    Symbol id;
    if (hp)
        dataP.AddPair(pid, hp->GetOnlineID()->ToString());
    else
        dataP.AddPair(pid, "N/A");
    SetDataPoint(dataP);
}

UpdateFitnessGoalJob::UpdateFitnessGoalJob(Hmx::Object *callback, HamProfile *hp)
    : RCJob("fitness/updatefitnessgoal/", callback) {
    DataPoint dataP;
    static Symbol pid("pid");
    static Symbol days_active_td("days_active_td");
    static Symbol cals_burned_td("cals_burned_td");
    if (hp) {
        int daysActiveTD = 0;
        int calsBurnedTD = 0;
        hp->GetFitnessGoalStatus(daysActiveTD, calsBurnedTD);
        dataP.AddPair(pid, hp->GetOnlineID()->ToString());
        dataP.AddPair(days_active_td, daysActiveTD);
        dataP.AddPair(cals_burned_td, calsBurnedTD);
    } else {
        dataP.AddPair(pid, "N/A");
    }
    SetDataPoint(dataP);
}
