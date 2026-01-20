#include "net_ham/FitnessGoalJobs.h"
#include "meta_ham/HamProfile.h"
#include "net/JsonUtils.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
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

void GetFitnessGoalJob::GetFitnessGoal(HamProfile *profile) {
    if (mResult == 1) {
        JsonObject *response = mJsonResponse;
        JsonConverter &reader = mJsonReader;
        if (response) {
            if (profile) {
                MILO_LOG(">>>>>>>>>> %s\'s fitness goal info.\n", profile->GetName());
            }

            JsonObject *getByName = reader.GetByName(response, "valid");
            if (!getByName || getByName->Bool()) {
                int startDay = 0;
                int startMonth = 0;
                int startYear = 0;
                int daysActive = 0;
                int calories = 0;
                int daysActiveTd = 0;
                int calsBurnedTd = 0;

                getByName = reader.GetByName(response, "start_day");
                if (getByName) {
                    startDay = getByName->Int();
                }
                getByName = mJsonReader.GetByName(response, "start_month");
                if (getByName) {
                    startMonth = getByName->Int();
                }
                getByName = mJsonReader.GetByName(response, "start_year");
                if (getByName) {
                    startYear = getByName->Int();
                }
                getByName = mJsonReader.GetByName(response, "days_active");
                if (getByName) {
                    daysActive = getByName->Int();
                }
                getByName = mJsonReader.GetByName(response, "calories");
                if (getByName) {
                    calories = getByName->Int();
                }
                getByName = mJsonReader.GetByName(response, "days_active_td");
                if (getByName) {
                    daysActiveTd = getByName->Int();
                }
                getByName = mJsonReader.GetByName(response, "cals_burned_td");
                if (getByName) {
                    calsBurnedTd = getByName->Int();
                }

                MILO_LOG(
                    ">>>>>>>>>> UTC start date: %i/%i/%i.\n",
                    startDay,
                    startMonth,
                    startYear
                );

                DateTime dt(startYear, startMonth, startDay, 0, 0, 0);
                dt.FromUtcToLocal();
                startDay = dt.Year();
                startMonth = dt.Month();
                MILO_LOG(
                    ">>>>>>>>>> local start date: %i/%i/%i.\n",
                    dt.mDay,
                    startMonth,
                    startDay
                );
                MILO_LOG(">>>>>>>>>> target days: %i\n", daysActive);
                MILO_LOG(">>>>>>>>>> target calories: %i\n", calories);
                MILO_LOG(">>>>>>>>>> days active: %i\n", daysActiveTd);
                MILO_LOG(">>>>>>>>>> calories burned: %i\n", calsBurnedTd);
                profile->SetFitnessGoal(
                    true,
                    dt.mDay,
                    dt.Month(),
                    dt.Year(),
                    daysActive,
                    calories,
                    daysActiveTd,
                    calsBurnedTd
                );
            } else {
                MILO_LOG(">>>>>>>>>> invalid fitness goal.\n");
                profile->SetFitnessGoal(false, 0, 0, 0, 0, 0, 0, 0);
            }
        }
    }
}
