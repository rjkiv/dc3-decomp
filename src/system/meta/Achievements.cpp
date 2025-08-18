#include "meta/Achievements.h"
#include "obj/Dir.h"

#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "os/ThreadCall.h"

Achievements *TheAchievements;
std::vector<XUSER_ACHIEVEMENT> Achievements::gThreadAchievements;

void Achievements::Terminate() { RELEASE(TheAchievements); }

BEGIN_HANDLERS(Achievements)
    HANDLE_ACTION(set_allow_achievements, SetAllowAchievements(_msg->Int(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

Achievements::Achievements() : unk2c(0), mAllowAchievements(true) {}

void Achievements::SubmitAchievementsCallback(int) { gThreadAchievements.clear(); }

void Achievements::Init() {
    TheAchievements = new Achievements();
    TheAchievements->SetName("achievements", ObjectDir::Main());
    PlatformInit();
}

void Achievements::Poll() {
    if (!mAchieved.empty() && gThreadAchievements.empty() && mAllowAchievements) {
        gThreadAchievements = mAchieved;
        mAchieved.clear();
        ThreadCall(SubmitAchievementsFunc, SubmitAchievementsCallback);
    }
}

void Achievements::Submit(int i1, Symbol s2, int i3) {
    if (ThePlatformMgr.IsPadNumSignedIn(i1)) {
        MILO_LOG("Achievement awarded: %s (id:%d, pad:%i)\n", s2, i3, i1);
        mAchieved.push_back(GetAchievementData(i1, i3));
    }
}
