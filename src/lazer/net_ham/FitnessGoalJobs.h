#pragma once
#include "meta_ham/HamProfile.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"

class GetFitnessGoalJob : public RCJob {
public:
    GetFitnessGoalJob(Hmx::Object *, char const *);
    void GetFitnessGoal(HamProfile *);
};

class SetFitnessGoalJob : public RCJob {
public:
    SetFitnessGoalJob(Hmx::Object *, HamProfile *);
};

class DeleteFitnessGoalJob : public RCJob {
public:
    DeleteFitnessGoalJob(Hmx::Object *, HamProfile *);
};

class UpdateFitnessGoalJob : public RCJob {
public:
    UpdateFitnessGoalJob(Hmx::Object *, HamProfile *);
};
